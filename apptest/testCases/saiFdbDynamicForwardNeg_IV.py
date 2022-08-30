import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiFdbDynamicForwardNeg_IV',
    'description' : 'TC to verify the dynamic learning - drop on the other vlan member (part-2)',
    'ingressPort' : ['30'],
    'egressPort' : ['31'],
    'vlan'       : '75',
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap3'],
    'count' : 0 ,            # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
vlan
sai_remove_vlan_member $memtap2
sai_remove_vlan_member $memtap3
sai_remove_vlan_member $memtap1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan $vlan75
back
fdb
sai_flush_fdb_entries 9288674231451648 SAI_FDB_FLUSH_ATTR_BV_ID,$vlan75
home
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:11",dst="00:00:11:00:11:00")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()
#packet_info = scapy_t.Ether(src="00:00:11:00:11:11",dst="00:00:11:00:11:00")/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
}
