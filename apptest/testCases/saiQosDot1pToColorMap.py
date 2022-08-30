#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosDot1pToColorMap',
    'description' : 'Verifies Dot1p to color Qos map',
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
qosmap
sai_create_qos_map 9288674231451648 [0,0:1,0:2,1:3,1:4,1:5,2:6,2:7,2] SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR > qosmap_id
sai_get_qos_map_attribute $qosmap_id SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST 8
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP $qosmap_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP 1
dump $qosmap_id
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP 0
back
qosmap
sai_remove_qos_map $qosmap_id
back
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'5629499534213121'
}
