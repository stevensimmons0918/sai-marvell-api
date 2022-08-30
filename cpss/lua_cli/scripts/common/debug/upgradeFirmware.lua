--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* upgradeFirmware.lua
--*
--* DESCRIPTION:
--*       Starts remote upgrade firmware of the Service CPU
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

--includes

--constants

  local updateStatus = nil

local function bit_and(a, b)
    local result = 0
    local bitval = 1
    while a > 0 and b > 0 do
      if a % 2 == 1 and b % 2 == 1 then -- test the rightmost bits
          result = result + bitval      -- set the current bit
      end
      bitval = bitval * 2 -- shift left
      a = math.floor(a/2) -- shift right
      b = math.floor(b/2)
    end
    return result
end

local function stringToHex(str)
   return (str:gsub('.', function (c)
       return string.format('%02X', string.byte(c))
    end))
end

-- ************************************************************************
---
--  receivePacket (logic copied from lua_cli/scripts/dxCh/traffic/RxTx.lua)
--        @description    receiving of packets
--
--        @param params - packet command parameters
--
--        @return         return the packet content if exist
--
function receivePacket(params)
    local rxQ,status,packetContent

    setRcv(nil,false)
    setRcv(params.port,true)

    rxQ = registerRx("rcvSinglePacket",{})
    status, packetContent = luaMsgQRecv(rxQ,params.timeout * 1000)
    unregisterRx("rcvSinglePacket",rxQ)

    setRcv(nil,false)
    return packetContent
end

-- ************************************************************************
---
--  upgradeFW
--        @description    Upgrade FW of the Service CPU using
--                        image located in RAMFS
--
--        @param params - The parameters
--
--        @return         true if there was no error otherwise false
--
local function upgradeFW(params)
  local result, values, devNum
  local status, err

  local str, e, fd

  local inBufSize       = 0
  local fileOfs         = 0
  local bufNum          = 0
  local bufSize         = 0
  local dataLen         = 0
  local bufBmp          = 3
  local bufNumStr, miStatus, imageSrc, opCodeHdr, pcktHdr
  local pcktHdrFixed     = "0000000000010000000000029988000000000010" -- packet header:
                                                                      -- MAC Destination (00.00.00.01)
                                                                      -- MAC Source(00.00.00.02)
                                                                      -- EthType (0x9988) two bytes padded
                                                                      -- opCode (0x10)

                                                                      -- massage number (N)
                                                                      -- device number (params.devNum)
  local receivedData, msgNum, retCode

  devNum = 0
  status = true

  if ((params.imageNum == nil) or (params.imageNum<1) or (params.imageNum>6))then
    print("Image number error")
    return false
  end
  bufBmp = params.imageNum

  if params.filename == nil then
    print("Image file name error.")
    return false
  end

  if ((params.sizeNum == nil) or ((params.sizeNum)%4 ~= 0)) then
    print("Input buffer size is invalid - must be align to 4")
    return false
  end

  inBufSize = params.sizeNum

  print("Loading " .. params.filename .. " .. \n")

  local fstat = fs.stat(params.filename)
  if fstat == nil then
    print("Error: Input file " .. tostring(params.filename) .. " not found.")
    return false
  end

  local fsize = fstat.st_size

  fd, e = fs.open(params["filename"], 'r')
  if fd == nil then
    print("failed to open file " .. params["filename"] .. ": ".. to_string(e))
    return false, e
  end

  -- iterate thru file
  while (fileOfs<fsize)  do
    bufSize=fsize-fileOfs;
    if (bufSize<=inBufSize) then
      dataLen = bufSize
      bufSize = bufSize+0x100 -- last block of file
    else
      dataLen = inBufSize
      bufSize = inBufSize
    end

    imageSrc = fs.read(fd, dataLen )

    if imageSrc == nil or imageSrc == "" then
      print("error reading file: " .. params["filename"])
      return false
    end

    -- upgrade MI firmware
    if params.port == nil then
        result, values = myGenWrapper("CpssGenericSrvCpuFWUpgrade",
                                      {
                                        { "IN", "GT_U8",  "devNum",   devNum  },
                                        { "IN", "GT_U8",  "bufBmp",   bufBmp  },
                                        { "IN", "GT_U32", "fileOfs",  fileOfs/inBufSize },
                                        { "IN", "GT_U32", "bufSize",  bufSize },
                                        { "IN", "string", "fwBufPtr", imageSrc},
                                        { "OUT","GT_U32", "miStatus", miStatus}
                                      })
    else
        -- upgrade MI firmware through FW_SDK
        -- packet must be aligned to 32 bits
        -- header for each buffer including: bootChannel opCode, bufBmp, bufNum & bufSize (12Bytes)
        devNum = string.format("%8x", bit_and(params.devNum,0xFFFFFFFF) )
        bufNum = fileOfs/inBufSize
        print("bufNum= " .. bufNum .. ", bufSize= " .. bufSize )

        bufNumStr = string.format("%8x", bit_and(bufNum,0xFFFFFFFF) )
        pcktHdr = pcktHdrFixed .. bufNumStr .. devNum
        pcktHdr = string.gsub(pcktHdr, " ", "0")
        opCodeHdr = string.format("%8x", bit_and((bufBmp * 2 ^ 16 ) + 0x10,0xFFFFFFFF) ) -- bufBmp & opCode=0x10
        opCodeHdr = opCodeHdr .. bufNumStr                                               -- bufNum
        opCodeHdr = opCodeHdr .. string.format("%8x", bit_and(bufSize,0xFFFFFFFF))       -- bufSize
        opCodeHdr = string.gsub(opCodeHdr, " ", "0")

        imageSrc = pcktHdr .. opCodeHdr .. stringToHex(imageSrc)

        -- sendPacket function from: lua_cli/scripts/dxCh/traffic/RxTx.lua
        result, values = sendPacket(params.port, imageSrc, string.len(imageSrc)/2)

        -- wait for packet before continue to next buffer (timeout: 30 seconds)
        -- for burning both images (bmp 3/6) need to wait for validation (crc) before the image copied to second offset
        params.timeout = 30
        receivedData = receivePacket(params)

        if receivedData == nil or receivedData == "" then
            print("Timeout error! no packets received\n")
            return false
        end

        receivedData = receivedData.packet.data
        msgNum = tonumber(string.sub(to_string(receivedData), 34,41),16)    -- sub string the message number (byte #5)
        retCode = tonumber(string.sub(to_string(receivedData), 42,49),16)   -- sub string the return code (byte #6)

        if msgNum ~= bufNum then
            print("Error - packet reurned with wrong message number #" .. to_string(msgNum) .. " buffNum - #" .. to_string(bufNum) .. "\n")
            return false
        end

        if retCode ~= 0 then
            print("Error - packet reurned with error code 0x" .. string.format("%2x", retCode).. "\n")
            return false
        end

    end

    if (result ~= 0) then
      print("Error: " .. to_string(returnCodes[result]) .. "\n")
      print("microInit code := " .. to_string(values.miStatus) .. "\n")
      return false
    end
    fileOfs = fileOfs + inBufSize

  end
  fs.close(fd)

  -- save update results
  updateStatus = 0
  return true
end

CLI_type_dict["deviceFW"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help = "Enter device number to upgrade ('0' for local)"
}

CLI_type_dict["imageFW"] = {
    checker = CLI_check_param_number,
    min=1,
    max=6,
    help = "Enter number of upgraded FW image: 1,2 or 3 for both images"
}

CLI_type_dict["bufSize"] = {
    checker = CLI_check_param_number,
    min=4,
    max=212,
    help = "Enter number of upgraded FW image: 1,2 or 3 for both images"
}

CLI_addCommand("debug", "load firmware", {
    help = "Transfer FirmWare located in RAMFS to Service CPU and burn it",
    func = upgradeFW,
    params={{ type="named",
        { format="imageNum %imageFW", name="imageNum", help="Number of upgraded FW image 1,2 or 3 for both images" },
        { format="imageName %filename", name="filename", help="booton.bin or extendedImage.bin file name" },
        { format="sizeNum %bufSize", name="sizeNum", requirements={"filename"}, help = "Size of buffer - (Min:32 Bytes, Max: 212 Bytes)" },
    }}
})

CLI_addCommand("debug", "load fw_sdk", {
    help = "Transfer FirmWare located in RAMFS to Service CPU through FW_SDK and burn it",
    func = upgradeFW,
    params={{ type="named",
        { format="port %port-range",name="port", help="The device and port number" },
        { format="devNum %deviceFW", name="devNum", help="fw device number to upgrade ('0' for local)" },
        { format="imageNum %imageFW", name="imageNum", help="Number of upgraded FW image 1,2 or 3 for both images (L1) 4,5 or 6 for both copies (costumer)" },
        { format="imageName %filename", name="filename", help="booton.bin or extendedImage.bin file name" },
        { format="sizeNum %bufSize", name="sizeNum", requirements={"filename"}, help = "Size of buffer - (Min:32 Bytes, Max: 212 Bytes)" },
        mandatory={"port"}
    }}
})
