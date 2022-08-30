#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosSymmetricPfc',
    'description' : 'Verifies Symmetric Pfc configuration',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'ingressTapIntf' : '',
    'egressTapIntf'  : [''],
    'count' : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
port
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL 1 > pfc_enable
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_MODE 0
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL 5
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL 1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL_MODE 0
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL $pfc_enable
back
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'5'
}
