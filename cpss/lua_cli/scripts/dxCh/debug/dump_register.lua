--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dump_register.lua
--*
--* DESCRIPTION:
--*       Dump registers commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function generic_unit_register_dump(params)

    local i, rc, values
    local fd, e
    local devices = (params.devNum == "all") and wrlDevList() or {params.devNum}

    -- Open File to dump the register data
    if(params["filename"] ~= nil) then
        if fs.fileexists(params["filename"]) then
            copy_file(params["filename"], params["filename"] .. ".bak")
        end
        fs.unlink(params["filename"])

        fd, e = fs.open(params["filename"], "w")
        if fd == nil then
            print("failed to open file " .. params["filename"] .. ": "..e)
            return false
        end
    end

    for _, devNum in ipairs(devices) do
        rc, values = myGenWrapper("cpssDxChDiagUnitRegsNumGet",{
            {"IN", "GT_U8",                "devNum",        devNum          },
            {"IN", "GT_PORT_GROUPS_BMP",   "portGroupsBmp", 0xFFFFFFFF      }, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
            {"IN", "CPSS_DXCH_UNIT_ENT",   "unitId",        params["unitId"]},
            {"OUT","GT_U32",               "regsNumPtr"}
        })
        if rc~=0 then
            print("cpssDxChDiagUnitRegsNumGet() failed: ".. tostring(rc))
            return rc
        end
        local regsNum = values["regsNumPtr"]
        local regsNumArrType = "GT_U32" .. "[" .. regsNum .. "]" -- GT_U32[regsNum]

        rc, values = myGenWrapper("cpssDxChDiagUnitRegsDump",{
            {"IN",   "GT_U8",              "devNum",        devNum          },
            {"IN",   "GT_PORT_GROUPS_BMP", "portGroupsBmp", 0xFFFFFFFF      }, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
            {"IN",   "CPSS_DXCH_UNIT_ENT", "unitId",        params["unitId"]},
            {"INOUT","GT_U32",             "regsNumPtr",    regsNum         },
            {"IN",   "GT_U8",              "offset",        devNum          },
            {"OUT",  regsNumArrType,       "regAddrArr"                     },
            {"OUT",  regsNumArrType,       "regDataArr"                     }
        })
        if rc~=0 then
            print("cpssDxChDiagUnitRegsDump() failed: ".. tostring(rc))
            return rc
        end

        -- Writting to File
        if(params["filename"] ~= nil) then
            fs.write(fd, string.format("\nUnit name : %s\n", string.sub(params["unitId"], 16, -3)))
            fs.write(fd, "\nRegister   :   Register")
            fs.write(fd, "\nAddress    :   Data")
            fs.write(fd, "\n-----------------------")

            for i=0,(regsNum-1) do
                fs.write(fd, string.format("\n0x%8.8x : 0x%8.8x",values.regAddrArr[i], values.regDataArr[i]))
            end
        else
            print(string.format("Unit name : %s", string.sub(params["unitId"], 16, -3)))
            print("Register   :   Register")
            print("Address    :   Data")
            print("-----------------------")

            for i=0,(regsNum-1) do
                print(string.format("0x%8.8x : 0x%8.8x",values.regAddrArr[i], values.regDataArr[i]))
            end
        end
    end -- Device Loop

    if(params["filename"] ~= nil) then
        -- Close the fd all the register dump wriiten for all devices.
        fs.close(fd)

        -- Copy file from the RAM-FS to the hard-drive
        if(cmdCopyFromRAMFStoDisk(params["filename"]) == -1) then
            print("Error: Unable to save file ".. params["filename"] .. " to disk\n")
            return false
        end
    end

    return 0;
end

--------------------------------------------------------------------------------
---- command registration: dump register
--------------------------------------------------------------------------------
CLI_addCommand("debug", "dump register", {
   func = generic_unit_register_dump,
   help = "Dump register",
   params={
         { type="named",
         { format="device %devID_all"   ,name="devNum",     help="The device number" },
         { format="unitId %unitId_all"  ,name="unitId",     help="unit name"},
         { format="file %filename"      ,name="filename",   help="File name to output register dump"},
         mandatory = {"unitId"}
        }
    }
})

