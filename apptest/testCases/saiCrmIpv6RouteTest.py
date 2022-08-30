#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiCrmIpv6RouteTest',
    'description' : 'TC to check the crm counter after Ipv6 route entry',
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
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY 0 > max_count
back
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef
back
route
sai_create_route_entry 9288674231451648 $vr_id 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:1 ff:ff:ff:ff:ff:ff:ff:ff:00:00:00:00:00:00:00:00
sai_create_route_entry 9288674231451648 $vr_id 1 20:01:D:B8:0:5:0:5:0:0:0:0:0:0:0:1 ff:ff:ff:ff:ff:ff:ff:ff:00:00:00:00:00:00:00:00
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY 0
back
route
sai_remove_route_entry 9288674231451648 $vr_id 1 20:01:D:B8:0:5:0:5:0:0:0:0:0:0:0:1 ff:ff:ff:ff:ff:ff:ff:ff:00:00:00:00:00:00:00:00
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id 1 20:01:D:B8:0:4:0:4:0:0:0:0:0:0:0:1 ff:ff:ff:ff:ff:ff:ff:ff:00:00:00:00:00:00:00:00
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY 0
back
virtualrouter
sai_remove_virtual_router $vr_id
home
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count -1',
}

