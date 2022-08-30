import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiFdbDynamicFwd_I',
    'description' : 'TC to verify the dynamic learning (part-1)',
    'ingressPort' : ['29'],
    'egressPort' : ['30','31'],
    'vlan'       : '75',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2','tap3'],
    'count' : 0            # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218079 1 > memtap3 
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
alter_create_mode 0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
}

