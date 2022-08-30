import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L3Packet',
    'tcName'         : 'saiPortStatsIpv6_I',
    'description'    : 'Port statistics for V6 packets',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'vlan'       : '75',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'pktAction' : 'FORWARD',
    'count'          : 0,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
port
sai_clear_port_stats 281474976710685 SAI_PORT_STAT_IPV6_IN_RECEIVES
sai_clear_port_stats 281474976710686 SAI_PORT_STAT_IPV6_OUT_OCTETS
back
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IPv6(src="0200:0001:0:0:0:0:0:0008",dst="0200:0001:0:0:0:0:0:0009")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
}