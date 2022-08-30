import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3VerifyVrfUnkL3V6MC',
    'description' : 'TC to verify if basic VRF Unknown L3 V6 MC pkt action',
    'ingressPort' : ['30'],
    'egressPort' : [],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : '',
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
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id2
sai_set_virtual_router_attribute $vr_id2 SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:bb
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION SAI_PACKET_ACTION_COPY

back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id >rif_id_2
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id >rif_id_1
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE 1
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE 0
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE 1

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
routerinterface
sai_remove_router_interface $rif_id_1
sai_remove_router_interface $rif_id_2
back
virtualrouter
sai_remove_virtual_router $vr_id2
sai_remove_virtual_router $vr_id
back

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="33:33:00:00:00:1")/scapy_t.IPv6(dst="ff02:0:0:0:0:0:0:0001", src="3ffe:0502:4819:0:0:0:0:0")
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
        'expect1':'',
}
