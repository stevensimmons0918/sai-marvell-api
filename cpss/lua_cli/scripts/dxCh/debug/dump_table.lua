--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dump_table.lua
--*
--* DESCRIPTION:
--*       Dump table command
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function generic_table_dump(params)

    local i, j, rc, values
    local fd, e
    local devices = (params.devNum == "all") and wrlDevList() or {params.devNum}

    -- Open File to dump the table data
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
        -- Calculate entry size in words
        rc, values = myGenWrapper("cpssDxChDiagTableInfoGet",{
            {"IN",      "GT_U8",                "devNum",       devNum                  },
            {"IN",      "CPSS_DXCH_TABLE_ENT",  "tableType",    params["tableType"]     },
            {"OUT",     "GT_U32",               "numEntriesPtr"                         },
            {"OUT",     "GT_U32",               "entrySizePtr"                          }
           })
        if rc~=0 then
            print("cpssDxChDiagTableInfoGet() failed: ".. tostring(rc))
            return rc
        end

        local numEntries = values["numEntriesPtr"]
        local entrySize = values["entrySizePtr"]
        local tableBufferArrType = "GT_U32" .. "[" .. entrySize .. "]" -- GT_U32[entrySize]

        -- Writting to File
        if(params["filename"] ~= nil) then
            fs.write(fd, string.format("\nNum of Entries: %d\n", numEntries))
            fs.write(fd, string.format("\nEntry Size(in 32 bit word): %d\n", entrySize))
            fs.write(fd, "\nTable      :   Table")
            fs.write(fd, "\nAddress    :   Data")
            fs.write(fd, "\n-----------------------\n")

        else
            io.write(string.format("\nNumber of Entries: %d", numEntries))
            io.write(string.format("\nEntry Size(in 32 bit word): %d\n", entrySize))
            io.write("\nTable   :   Table")
            io.write("\nAddress :   Data")
            io.write("\n---------------------\n")
        end

        for i=0,(numEntries-1) do
            rc, values = myGenWrapper("cpssDxChDiagTableDump",{
                {"IN",   "GT_U8",                "devNum",          devNum             },
                {"IN",   "GT_PORT_GROUPS_BMP",   "portGroupsBmp",   0xFFFFFFFF         }, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                {"IN",   "CPSS_DXCH_TABLE_ENT",  "tableType",       params["tableType"]},
                {"IN",   "GT_BOOL",              "tableValid",      GT_FALSE           },
                {"INOUT","GT_U32",               "bufferLengthPtr", entrySize          },
                {"IN",   "GT_U32",               "firstEntry",      i                  },
                {"IN",   "GT_U32",               "lastEntry",       i                  },
                {"OUT",  tableBufferArrType,     "tableBufferArr"                      },
                {"OUT",  "GT_U32",               "entrySizePtr"                        }
            })
            if rc~=0 then
                print("cpssDxChDiagTableDump() failed: ".. tostring(rc))
                return rc
            end

            -- Writting to File
            if(params["filename"] ~= nil) then
                fs.write(fd, string.format("%d\t: ", i))
                for j=0,(entrySize-1) do
                    fs.write(fd, string.format("0x%8.8x ", values.tableBufferArr[j]))
                    if((j+1)%4 == 0) then
                        fs.write(fd, "\n\t  ")
                    end
                end
                fs.write(fd, "\n")
            else
                io.write(string.format("%d\t: ",i))
                for j=0,(entrySize-1) do
                    io.write(string.format("0x%8.8x ", values.tableBufferArr[j]))
                    if((j+1)%4 == 0) then
                        io.write("\n\t  ")
                    end
                end
                io.write("\n")
            end
        end
    end -- Device Loop

    if(params["filename"] ~= nil) then
        -- Close the fd all the table dump written for all devices.
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
---- command registration: dump table
--------------------------------------------------------------------------------
CLI_addCommand("debug", "dump table", {
   func = generic_table_dump,
   help = "Dump table",
   params={
         { type="named",
         { format="device %devID_all"         ,name="devNum",     help="The device number"              },
         { format="tableType %tableType_all"  ,name="tableType",  help="Table name"                     },
         { format="file %filename"            ,name="filename",   help="File name to output table dump" },
         mandatory = {"tableType"}
        }
    }
})


