--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* falcon_reg_addr.lua
--*
--* DESCRIPTION:
--*        A file to allow 2 functions: read/write to bar0 or bar2.
--*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
--*        when access to BAR0 caller give the address (offset) within the BAR0.
--*        when access to BAR2 caller give the 'Cider' address within the switch.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function lua_falcon_bar_reg_read(params,bar)
    if not params.pciDev  then params.pciDev = "0" end
    if not params.pciFunc then params.pciFunc = "0" end
	
    local string_to_execute = "falcon_bar_reg_read " .. 
        params.pciBus .. " " ..
        params.pciDev .. " " ..
        params.pciFunc .. " " ..
        tostring(bar)  .. " " ..
        tostring(params.regAddr)
	
    --the C function print the value
    luaShellExecute(string_to_execute)
end
local function lua_falcon_bar_reg_write(params,bar)

    if not params.pciDev  then params.pciDev = "0" end
    if not params.pciFunc then params.pciFunc = "0" end

    local string_to_execute = "falcon_bar_reg_write " .. 
        params.pciBus .. " " ..
        params.pciDev .. " " ..
        params.pciFunc .. " " ..
        tostring(bar)  .. " " ..
        tostring(params.regAddr) .. " " ..
        tostring(params.value)

    luaShellExecute(string_to_execute)
end

--------------------------------------------------------------------------------
---- command registration: falcon-bar0-read
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-bar0-read", {
   func = function (params)
            lua_falcon_bar_reg_read(params,0)
        end,
   help = "Function to read from BAR0 (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="pciBus %d"   ,name="pciBus",     help="pciBus" },
         { format="pciDev %d"   ,name="pciDev",     help="pciDev" },
         { format="pciFunc %d"   ,name="pciFunc",    help="pciFunc" },
         { format="regAddr %register_address"   ,name="regAddr",    help="regAddr (in HEX)" },
         mandatory = {"pciBus","regAddr"}
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: sip6-bar2-read
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-bar2-read", {
   func = function (params)
            lua_falcon_bar_reg_read(params,2)
        end,
   help = "Function to read from BAR2 (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="pciBus %d"   ,name="pciBus",     help="pciBus" },
         { format="pciDev %d"   ,name="pciDev",     help="pciDev" },
         { format="pciFunc %d"   ,name="pciFunc",    help="pciFunc" },
         { format="regAddr %register_address"   ,name="regAddr",    help="regAddr (in HEX)" },
         mandatory = {"pciBus","regAddr"}
        }
    }
})
--------------------------------------------------------------------------------
---- command registration: falcon-bar0-write
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-bar0-write", {
   func = function (params)
            lua_falcon_bar_reg_write(params,0)
        end,
   help = "Function to write to BAR0 (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="pciBus %d"   ,name="pciBus",     help="pciBus" },
         { format="pciDev %d"   ,name="pciDev",     help="pciDev" },
         { format="pciFunc %d"   ,name="pciFunc",    help="pciFunc" },
         { format="regAddr %register_address"   ,name="regAddr",    help="regAddr (in HEX)" },
         { format="value %GT_U32_hex"   ,name="value",      help="value (in HEX)" },
         mandatory = {"pciBus","regAddr","value"}
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: sip6-bar2-write
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-bar2-write", {
   func = function (params)
            lua_falcon_bar_reg_write(params,2)
        end,
   help = "Function to write to BAR2 (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="pciBus %d"   ,name="pciBus",     help="pciBus" },
         { format="pciDev %d"   ,name="pciDev",     help="pciDev" },
         { format="pciFunc %d"   ,name="pciFunc",    help="pciFunc" },
         { format="regAddr %register_address"   ,name="regAddr",    help="regAddr (in HEX)" },
         { format="value %GT_U32_hex"   ,name="value",      help="value (in HEX)" },
         mandatory = {"pciBus","regAddr","value"}
        }
    }
})

local function lua_dma_write(params)

    local string_to_execute = "debug_DMA_write " .. 
        tostring(params.offset) .. " " ..
        tostring(params.value)

    luaShellExecute(string_to_execute)
end
local function lua_dma_read(params)

    local string_to_execute = "debug_DMA_read " .. 
        tostring(params.offset)

    luaShellExecute(string_to_execute)
end
--------------------------------------------------------------------------------
---- command registration: sip6-dma-read
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-dma-read", {
   func = lua_dma_read,
   help = "Function to read from DMA (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="offset %register_address"   ,name="offset",    help="offset from start of DMA (in HEX)" },
         mandatory = {"offset"}
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: sip6-dma-write
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-dma-write", {
   func = lua_dma_write,
   help = "Function to write to DMA (sip6 device) (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="offset %register_address"   ,name="offset",    help="offset from start of DMA (in HEX)" },
         { format="value %GT_U32_hex"   ,name="value",      help="value (in HEX)" },
         mandatory = {"offset","value"}
        }
    }
})

local function lua_dma_pci_params_set(params,bar)

    if not params.pciDev  then params.pciDev = "0" end
    if not params.pciFunc then params.pciFunc = "0" end
    if not params.pciDomain then params.pciDomain = "0" end
	

    local string_to_execute = "debug_DMA_set_pci_params " .. 
        params.pciDomain .. " " ..
        params.pciBus .. " " ..
        params.pciDev .. " " ..
        params.pciFunc 

    luaShellExecute(string_to_execute)
end

--------------------------------------------------------------------------------
---- command registration: sip6-bar2-write
--------------------------------------------------------------------------------
CLI_addCommand("debug", "sip6-dma-pci-params-set", {
   func = lua_dma_pci_params_set,
   help = "Function to set PCI parameters , before doing DMA first allocation (prior to cpssInitSystem)",
   params={
         { type="named",
         { format="pciBus %d"   ,name="pciBus",     help="pciBus" },
         { format="pciDev %d"   ,name="pciDev",     help="pciDev" },
         { format="pciFunc %d"   ,name="pciFunc",    help="pciFunc" },
         { format="pciDomain %d"   ,name="pciDomain",    help="pciDomain" },
         mandatory = {"pciBus"}
        }
    }
})
