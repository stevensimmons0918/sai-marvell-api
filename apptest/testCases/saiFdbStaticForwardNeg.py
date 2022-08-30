import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiFdbVlanPortFlood',
    'description' : 'TC to verify the fdb forwarding - the traffic should not get flooded to other vlan port -- verifying on the other vlan member',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'vlan'       : '75',
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : [],
    'count' : 0,            # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
back
fdb
sai_create_fdb_entry 9288674231451648 [00:00:11:00:11:23] $vlan75 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218076 SAI_FDB_ENTRY_TYPE_STATIC
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
fdb
sai_remove_fdb_entry 9288674231451648 [00:00:11:00:11:23] $vlan75
back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan_member $memtap2
sai_remove_vlan $vlan75
back
fdb
sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan75
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
}

