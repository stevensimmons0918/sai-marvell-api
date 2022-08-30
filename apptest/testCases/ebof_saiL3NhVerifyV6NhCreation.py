#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'ebof_saiNhVerifyV6NhCreation',
    'description' : 'TC to verify if the V6 NH created and programmed properly',
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
sai_set_port_attribute 281474976710660 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710676 SAI_PORT_ATTR_ADMIN_STATE 1
home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710660 $vr_id >rif_id_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710676 $vr_id >rif_id_2
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:00:00:01
sai_set_router_interface_attribute $rif_id_2 SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS 00:00:01:00:00:02
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_id_2 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1 00:02:03:04:05:06
back

virtualrouter
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef
back

nexthop
sai_create_next_hop 9288674231451648 0 $rif_id_2 SAI_NEXT_HOP_TYPE_IP 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1 >next_hop_id_out1
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
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_id_2 1 10:01:0:0:0:0:0:0:0:0:0:0:0:0:0:1
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
       'expect1':'16:1:0:0:0:0:0:0:0:0:0:0:0:0:0:1',
}
