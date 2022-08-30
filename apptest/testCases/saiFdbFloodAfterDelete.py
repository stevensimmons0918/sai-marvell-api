import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiFdbFloodAfterDelete',
    'description' : 'TC to verify the l2 flooding after deleting the fdb entry',
    'ingressPort' : ['30'],
    'egressPort' : ['29','31'],
    'vlan'       : '75',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap1','tap3'],
    'count' : 2             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218079 1 > memtap3 
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
back
fdb
sai_create_fdb_entry 9288674231451648 [00:00:11:00:11:23] $vlan75 SAI_PACKET_ACTION_FORWARD 0 0.0.0.0 16325548649218079 SAI_FDB_ENTRY_TYPE_STATIC
sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan75
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $memtap3
sai_remove_vlan_member $memtap1
sai_remove_vlan_member $memtap2
sai_remove_vlan $vlan75
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap1': [<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4''',
'expect2':'''
'tap3': [<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4
'''
}

