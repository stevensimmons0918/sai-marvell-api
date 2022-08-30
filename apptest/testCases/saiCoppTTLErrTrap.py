import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'V4Packet',
    'tcName' : 'saiCoppTTLErrTrap',
    'description' : 'TC to verify that the IPv4 packet with TTL = 1 is trapped when the packet action is set to trap',
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
sai_create_hostif_trap 9288674231451648 SAI_HOSTIF_TRAP_TYPE_TTL_ERROR SAI_PACKET_ACTION_TRAP >trp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP $tgrp
sai_set_hostif_trap_group_attribute $tgrp SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE 7
sai_set_hostif_trap_group_attribute $tgrp SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER $plcr1
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION SAI_PACKET_ACTION_TRAP

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id6
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id6 >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id6 >rif_id_2
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
sai_set_router_interface_attribute $rif_id_2 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02


back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_1 0 10.1.1.20 00:02:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $rif_id_1 SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1

back
route
sai_create_route 9288674231451648 $vr_id6 0 55.50.2.0 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id6 0 55.50.2.0 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $next_hop_id_out1

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
sai_remove_hostif_trap_group $tgrp
sai_remove_hostif_table_entry $hostiftab 
home
sai
policer
sai_remove_policer $plcr1
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id6 0 55.50.2.0 [255.255.255.0]
back
nexthop
sai_remove_next_hop $next_hop_id_out1
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_1 0 10.1.1.20
back

routerinterface
sai_remove_router_interface $rif_id_1
sai_remove_router_interface $rif_id_2
home
sai
virtualrouter
sai_remove_virtual_router $vr_id6
back
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="55.50.2.10", src="50.50.1.10",ttl=1)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
}
