#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmIpv6NeighborTest',
    'description' : 'TC to check the crm counter for Ipv6 Neighbor',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.`

tcProgramStr = '''
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY 0 > max_count
back
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710662 $vr_id > rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710663 $vr_id > rif_i_prt_02
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:2 00:04:04:04:04:49
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_02 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:3 00:04:04:04:04:50
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY 0
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_02 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:3 
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:2
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY 0
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
sai_remove_router_interface $rif_i_prt_02
back
virtualrouter
sai_remove_virtual_router $vr_id
home

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count - 1', #entry; V6 Nbr=2entries. Not for M0, maxArp < maxFdb
}

