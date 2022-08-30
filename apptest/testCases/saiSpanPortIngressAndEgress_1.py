import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiSpanPortIngressAndEgress_1',
    'description' : 'TC to verify port based ingress and egress SPAN',
    'ingressPort' : ['28'],
    'egressPort' : ['29','30'],
    'vlan'       : '75',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,   # expected data count
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
back
mirror
sai_create_mirror_session 9288674231451648 0 0.0.0.0 0 0.0.0.0 0 0 0 281474976710686 0 0 0 0 0 > mirror_session0
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_MIRROR_SESSION $mirror_session0
sai_get_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_MIRROR_SESSION 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION $mirror_session0
sai_get_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 1
back
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_MIRROR_SESSION 0
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 0
back
mirror
sai_remove_mirror_session $mirror_session0
back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP(src="195.169.0.20", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       #'expect1':'''
#'tap1':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.20 dst=195.169.0.18''',
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.20 dst=195.169.0.18'''
}
