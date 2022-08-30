-- TCP options: |Hrd Len| Rsvd | URG | ACK | PSH | RST | SYN | FIN
--                4b      6b     1b    1b    1b    1b    1b    1b


-------------------------------------------------------------------------------------
local unkownUcPacketContent1 =
        "000000003411"..             --mac da 00:00:00:00:34:02
        "000000000055"..             --mac sa 00:00:00:00:00:55
        "0800"..                     --ethertype
        "4500002E0000000040ff74cb".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "02020204"..                 --source ip 02.02.02.04
        "01010000"..                 --destination ip 01.01.00.00
        "000102030405060708090A"..
        "0B0C0D0E0F101112131415"..
        "161718191A1B1C1D1E1F20"..
        "2122232425262728292A2B2"..
        "C2D2E2F0000000000000000"    --payload

local unkownUcPacketContent2 =
        "000000003422"..             --mac da 00:00:00:00:34:02
        "000000001144"..             --mac sa 00:00:00:00:11:44
        "0800"..                     --ethertype
        "4500002E0000000040ff74cb".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "02020204"..                 --source ip 02.02.02.04
        "01010000"..                 --destination ip 01.01.00.00
        "000102030405060708090A"..
        "0B0C0D0E0F101112131415"..
        "161718191A1B1C1D1E1F20"..
        "2122232425262728292A2B2"..
        "C2D2E2F0000000000000000"    --payload

local unkownUcPacketContent3 =
        "ffffffffffff"..             --mac da ff:ff:ff:ff:ff:ff
        "000000001144"..             --mac sa 00:00:00:00:11:44
        "0800"..                     --ethertype
        "4500002E0000000040ff74cb".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "02020204"..                 --source ip 02.02.02.04
        "01010000"..                 --destination ip 01.01.00.00
        "000102030405060708090A"..
        "0B0C0D0E0F101112131415"..
        "161718191A1B1C1D1E1F20"..
        "2122232425262728292A2B2"..
        "C2D2E2F0000000000000000"    --payload


return { unkownUcPacket1 = unkownUcPacketContent1 ,unkownUcPacket2 = unkownUcPacketContent2 ,unkownUcPacket3 = unkownUcPacketContent3}
