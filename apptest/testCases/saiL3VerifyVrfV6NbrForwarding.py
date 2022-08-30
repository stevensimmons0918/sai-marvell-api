import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3VerifyVrfV6NbrForwarding',
    'description' : 'TC to verify if basic Host route forwarding works through V6 Neigbor',
    'ingressPort' : ['30'],
    'egressPort' : ['29'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap1'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id2
sai_set_virtual_router_attribute $vr_id2 SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:bb
back

routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id >rif_id_2

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_1 1 20:01:01:01:02:02:0:0:0:0:0:0:0:0:0:0 00:02:03:04:05:06
back
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_1 1 20:01:01:01:02:02:0:0:0:0:0:0:0:0:0:0 00:02:03:04:05:06
back
routerinterface
sai_remove_router_interface $rif_id_1
sai_remove_router_interface $rif_id_2
back
virtualrouter
sai_remove_virtual_router $vr_id2
sai_remove_virtual_router $vr_id
back

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:bb")/scapy_t.IPv6(dst="2001:0101:0202:0:0:0:0:0", src="3ffe:0502:4819:0:0:0:0:0")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap1': [<Ether  dst=00:02:03:04:05:06 src=00:00:01:11:22:bb type=IPv6 |<IPv6  version=6 tc=0 fl=0 plen=0 nh=No Next Header hlim=63 src=3ffe:502:4819:: dst=2001:101:202:: |
'''
}
