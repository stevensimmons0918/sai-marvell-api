import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L2Packet',
    'tcName' : 'saiPortLoopbackModeMac',
    'description' : 'TC to verify that the loopbackMode of Mac can be set.',
    'ingressPort' : ['29'],
    'egressPort' : ['29'],
    'vlan' : '10',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap1'],
    'count': 1,         # number of expected data
}

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 10 > vlan_id_10
sai_create_vlan_member 9288674231451648 $vlan_id_10 16325548649218077 > vmem29
sai_create_vlan_member 9288674231451648 $vlan_id_10 16325548649218078 > vmem30

back
port
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE 2
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1 1
'''

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem29
sai_remove_vlan_member $vmem30
sai_remove_vlan $vlan_id_10
back
port
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE 0
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="ff:ff:ff:ff:ff:ff")/scapy_t.IP(src="195.169.0.22", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'''
}
