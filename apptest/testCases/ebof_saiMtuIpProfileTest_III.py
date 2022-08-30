#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'ebof_saiMtuIpProfileTest',
    'description' : 'TC to check the MTU',
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
routerinterface
sai_set_router_interface_attribute $rif_id_1 SAI_ROUTER_INTERFACE_ATTR_MTU 1514
sai_set_router_interface_attribute $rif_id_2 SAI_ROUTER_INTERFACE_ATTR_MTU 128
sai_set_router_interface_attribute $rif_id_3 SAI_ROUTER_INTERFACE_ATTR_MTU 5000
sai_set_router_interface_attribute $rif_id_4 SAI_ROUTER_INTERFACE_ATTR_MTU 384
sai_set_router_interface_attribute $rif_id_5 SAI_ROUTER_INTERFACE_ATTR_MTU 512
sai_set_router_interface_attribute $rif_id_6 SAI_ROUTER_INTERFACE_ATTR_MTU 640
sai_set_router_interface_attribute $rif_id_7 SAI_ROUTER_INTERFACE_ATTR_MTU 9000
sai_get_router_interface_attribute $rif_id_3 SAI_ROUTER_INTERFACE_ATTR_MTU 1
'''




#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'5000',
}

