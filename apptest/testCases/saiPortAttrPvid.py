import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L2Packet',
    'tcName' : 'saiPortAttrPvid',
    'description' : 'TC to verify that the PVID can be set to port correctly and takes effect.',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'vlan' : '400',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'count': 1,         # number of expected data
}

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 400 > vlan400
sai_create_vlan 9288674231451648 401 > vlan401
sai_create_vlan_member 9288674231451648 $vlan400 16325548649218077 1 > vmem29
sai_create_vlan_member 9288674231451648 $vlan401 16325548649218077 1 > vmem30
sai_create_vlan_member 9288674231451648 $vlan400 16325548649218078 1 > vmem31
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_PORT_VLAN_ID 400
'''

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem29
sai_remove_vlan_member $vmem30
sai_remove_vlan_member $vmem31
sai_remove_vlan $vlan400
sai_remove_vlan $vlan401
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_PORT_VLAN_ID 1
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="ff:ff:ff:ff:ff:ff")/scapy_t.IP(src="195.169.0.22", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'''
}
