#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMtuConfigTest',
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
#home
#xps
#mtuprofile
#xpsMtuGetMtuProfile 0 1 > $defaultMtu

home
sai
port
#sai_get_port_attribute 281474976710657 SAI_PORT_ATTR_MTU 1

sai_set_port_attribute 281474976710657 SAI_PORT_ATTR_MTU 512
sai_set_port_attribute 281474976710658 SAI_PORT_ATTR_MTU 512
sai_set_port_attribute 281474976710659 SAI_PORT_ATTR_MTU 1024
sai_set_port_attribute 281474976710660 SAI_PORT_ATTR_MTU 2048

sai_get_port_attribute 281474976710657 SAI_PORT_ATTR_MTU 1
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
       'expect1':'512',
       #'expect2':'1514',
}

