import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'DHCP V6 Packet',
    'tcName' : 'saiCoppDhcpTrapV6',
    'description' : 'TC to verify that the DHCPV6 packet is trapped when the packet action is set to trap',
    'ingressPort' : ['30'],
    'egressPort' : [],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''

home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
hostif
sai_create_hostif_table_entry 9288674231451648 SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD 0 SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT 0 >hostiftab

home
sai
policer
sai_create_policer 9288674231451648 SAI_METER_TYPE_PACKETS SAI_POLICER_MODE_TR_TCM SAI_POLICER_COLOR_SOURCE_BLIND > plcr1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_CIR 600
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_CBS 100
sai_get_policer_attribute $plcr1 SAI_POLICER_ATTR_CIR 800
sai_get_policer_attribute $plcr1 SAI_POLICER_ATTR_CBS 200
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_GREEN_PACKET_ACTION 1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_YELLOW_PACKET_ACTION 1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_RED_PACKET_ACTION 1
back

home
sai
hostif
sai_create_hostif_trap_group 9288674231451648 >tgrp
sai_set_hostif_trap_group_attribute $tgrp SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE 7
sai_set_hostif_trap_group_attribute $tgrp SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER $plcr1

home
sai
hostif
sai_create_hostif_trap 9288674231451648 SAI_HOSTIF_TRAP_TYPE_DHCPV6_L2 SAI_PACKET_ACTION_TRAP >trp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP $tgrp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION SAI_PACKET_ACTION_TRAP
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
policer
sai_get_policer_stats $plcr1 0
back
home
sai
hostif
sai_remove_hostif_trap $trp
sai_remove_hostif_table_entry $hostiftab
sai_remove_hostif_trap_group $tgrp
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="33:33:00:01:00:02",type=0x86dd)/scapy_t.IPv6(dst="ff02:0:0:0:0:0:1:2", src="3ffe:0502:4819:0:0:0:0:0")/scapy_t.UDP(sport=546,dport=547)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
}

