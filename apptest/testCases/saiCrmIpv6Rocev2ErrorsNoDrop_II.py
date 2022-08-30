import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L4Packet',
    'tcName' : 'saiCrmIpv6Rocev2ErrorsNoDrop',
    'description' : 'Check ROCEv2 packets aren''t dropped',
    'ingressPort' : ['29'],
    'egressPort' : ['28'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap0'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
back
vlan
sai_create_vlan 9288674231451648 100 > vlan100
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218076 1 > vmem28
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218077 1 > vmem29
alter_create_mode 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem28
sai_remove_vlan_member $vmem29
sai_remove_vlan $vlan100
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:02:0a")/scapy_t.Dot1Q(vlan=100)/scapy_t.IPv6(dst="100::100", src="200::200")/scapy_t.UDP(sport=4000,dport=4791)/scapy_t.Raw("\x1100000000000\x6312345")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
       'tap0':[<Ether  dst=00:00:01:11:02:0a src=00:00:11:00:11:23 type=IPv6 |<IPv6  version=6 tc=0 fl=0 plen=26 nh=UDP hlim=64 src=200::200 dst=100::100 |<UDP  sport=4000 dport=4791 len=26 chksum=0xba9 |
        '''
}
