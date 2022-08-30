#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiPortAttrPvid2',
    'description' : 'setPvid and getPvid',
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
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_PORT_VLAN_ID 400
sai_get_port_attribute 281474976710684 SAI_PORT_ATTR_PORT_VLAN_ID 1
'''




#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_PORT_VLAN_ID 1
home
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'400',
}

