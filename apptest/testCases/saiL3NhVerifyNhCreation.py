#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiNhVerifyNhCreation',
    'description' : 'TC to verify if the NH created and programmed properly',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710658 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710675 SAI_PORT_ATTR_ADMIN_STATE 1
home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710658 $vr_id >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710675 $vr_id >rif_id_2
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:00:00:01
sai_set_router_interface_attribute $rif_id_2 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:00:00:02
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_2 0 10.1.1.1 01:01:01:01:01:01
sai_create_neighbor_entry 9288674231451648 $rif_id_1 0 20.1.1.1 02:02:02:02:02:02
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_id_2 SAI_NEXT_HOP_TYPE_IP 0 10.1.1.1 >next_hop_id_out1
sai_create_next_hop 9288674231451648 0 $rif_id_1 SAI_NEXT_HOP_TYPE_IP 0 20.1.1.1 >next_hop_id_out2
sai_get_next_hop_attribute $next_hop_id_out1 SAI_NEXT_HOP_ATTR_IP 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
nexthop
sai_remove_next_hop $next_hop_id_out1
sai_remove_next_hop $next_hop_id_out2
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_2 SAI_IP_ADDR_FAMILY_IPV4 10.1.1.1
sai_remove_neighbor_entry 9288674231451648 $rif_id_1 SAI_IP_ADDR_FAMILY_IPV4 20.1.1.1
back
routerinterface
sai_remove_router_interface $rif_id_1
sai_remove_router_interface $rif_id_2
back
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'10.1.1.1',
}
