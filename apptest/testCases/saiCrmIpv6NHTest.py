#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmIpv6NHTest',
    'description' : 'TC to check the crm counter for Ipv6 Next Hop',
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
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEXTHOP_ENTRY 0 > max_count
back
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710658 $vr_id > rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710659 $vr_id > rif_i_prt_02
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 [20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:2] > nh0
sai_create_next_hop 9288674231451648 0 $rif_i_prt_02 SAI_NEXT_HOP_TYPE_IP 0 [20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:3] > nh1
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEXTHOP_ENTRY 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count',
}

