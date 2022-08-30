import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiFdbLearnMode.py',
    'description'    : 'TC to verify FDB Learn mode Trap',
    'ingressPort'    : ['29'],
    'egressPort'     : [],
    'vlan'           : '75',
    'pktAction'      : 'TRAP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : '',
    'count'          : 1 ,             # expected data count
    'acl_counter'    : [] #only for acl cases
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
bridge
sai_set_bridge_port_attribute 16325548649218077 SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE 0
back

hostif
sai_create_hostif_trap_group 9288674231451648 >tgrp
sai_set_hostif_trap_group_attribute $tgrp SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE 3
sai_create_hostif_trap 9288674231451648 SAI_HOSTIF_TRAP_TYPE_EAPOL SAI_PACKET_ACTION_TRAP >trp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP $tgrp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION SAI_PACKET_ACTION_TRAP
sai_create_hostif_table_entry 9288674231451648 SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD 0 SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT 0 >hostiftab
home

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
bridge
sai_set_bridge_port_attribute 16325548649218077 SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE 2
home
sai
hostif
sai_remove_hostif_trap $trp
sai_remove_hostif_table_entry $hostiftab
sai_remove_hostif_trap_group $tgrp
home
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="01:80:C2:00:00:03")/scapy_t.Dot1Q(vlan =75, type=0x888E)/scapy_t.IP()/scapy_t.TCP()

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}

