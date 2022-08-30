--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shared-lib.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for shared-library
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--dofile("dxCh/debug/txq-sip6.lua")


local function map_pp(params)
  local res,val
  res, val = myGenWrapper("extDrvReInitDrv",{})
  if(res~=0)then
      print("Error "..res.." calling extDrvReInitDrv")
      return
  end

  print("Finished ok")
end


local function shlib_show_info(params)

  local res,val
  res, val  = myGenWrapper("prvOsHelperGlobalDbInfoGet",{
        {"OUT","GT_U32","myPidPtr"},
        {"OUT","GT_U32","clientNumPtr"},
        {"OUT","GT_BOOL","globalDbToSharedMemPtr"},
        {"OUT","GT_BOOL","ppMappingStageDonePtr"},
        {"OUT","GT_U32","initializerPidPtr"},
  })
  if(res~=0)then
      print("Error "..res.." calling prvOsHelperGlobalDbInfoGet")
      return

  else
   print("")
   print("My PID....................."..val.myPidPtr)
   print("Number of clients.........."..val.clientNumPtr)
   print("Global DB as shared memory."..tostring(val.globalDbToSharedMemPtr))
   print("PP mapping done............"..tostring(val.ppMappingStageDonePtr))
   print("Master process PID........."..val.initializerPidPtr)
  end

end

local function shlib_show_adress(params)

  local res,val
  res, val  = myGenWrapper("prvOsHelperGlobalDbAdressInfoGet",{
        {"OUT","GT_U32","dmaVirtualAddrLowPtr"},
        {"OUT","GT_U32","dmaVirtualAddrHighPtr"},
        {"OUT","GT_U32","dmaPhysAddrLowPtr"},
		{"OUT","GT_U32","dmaPhysAddrHighPtr"},
        {"OUT","GT_U32","globalSharedDbAddrLowPtr"},
        {"OUT","GT_U32","globalSharedDbAddrHiPtr"},
  })
  if(res~=0)then
      print("Error "..res.." calling prvOsHelperGlobalDbAdressInfoGet")
      return

  else
   print("")
   print("DMA virtual adress(low).....0x"..string.format("%08x",val.dmaVirtualAddrLowPtr))
   print("DMA virtual adress(high)....0x"..string.format("%08x",val.dmaVirtualAddrHighPtr))
   print("")
   print("DMA physical adress(low)....0x"..string.format("%08x",val.dmaPhysAddrLowPtr))
   print("DMA physical adress(high)...0x"..string.format("%08x",val.dmaPhysAddrHighPtr))
   print("")
   print("Global shared DB(high)......0x"..string.format("%08x",val.globalSharedDbAddrHiPtr))
   print("Global shared DB(low).......0x"..string.format("%08x",val.globalSharedDbAddrLowPtr))
  end

end

--------------------------------------------------------------------------------
---- command registration: shared-lib  map-pcie
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  pp-map", {
   func = map_pp,
   help = "Map PP to process memory space "
})


CLI_addHelp("debug", "shared-lib  show-info", "Show debug information")
--------------------------------------------------------------------------------
---- command registration: shared-lib  show-info
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  show-info", {
   func = shlib_show_info,
   help = "Show debug information "
})

CLI_addHelp("debug", "shared-lib  show-adress-map", "Show adresses used by shared lib")

--------------------------------------------------------------------------------
---- command registration: shared-lib  show-adress-map
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  show-adress-map", {
   func = shlib_show_adress,
   help = "Show adresses used by shared lib"
})


