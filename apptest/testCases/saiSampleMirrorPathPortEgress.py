import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiSampleMirrorPathPortEgress',
    'description' : 'TC to verify port based egress sample',
    'ingressPort' : ['28'],
    'egressPort'  : ['29','30'],
    'vlan'        : '75',
    'pktAction'   : 'FORWARD',
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
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 0 SAI_IP_ADDR_FAMILY_IPV4 0 0 0 SAI_MIRROR_SESSION_TYPE_SFLOW 281474976710686 0 0 0 0 0 > mirror_session0

home
sai
samplepacket
sai_create_samplepacket 9288674231451648 SAI_SAMPLEPACKET_MODE_SHARED SAI_SAMPLEPACKET_TYPE_MIRROR_SESSION 1 > smpl
sai_set_samplepacket_attribute $smpl SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE 1

home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_MIRROR_SESSION $mirror_session0
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE $smpl
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_MIRROR_SESSION 0
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE 0

back
samplepacket
sai_remove_samplepacket $smpl

back
mirror
sai_remove_mirror_session $mirror_session0

back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.44", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.44 dst=195.169.0.18
'''
}