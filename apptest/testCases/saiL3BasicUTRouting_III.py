#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiL3BasicUTRouting_III',
    'description' : 'Basic Routing',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 0,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
'''
#
#tcFlushStr: This string contains chain of saiShell commands to be used for get/flush specific
# networking scenario

tcFlushStr = '''
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back
nexthop
sai_remove_next_hop $nh0
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
back
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':''
}

