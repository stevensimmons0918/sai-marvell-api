#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosTcDpToDot1pMap',
    'description' : 'Verifies Tc, dp to dot1p Qos map',
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
sai_create_qos_map 9288674231451648 [0,0,7:0,1,7:0,2,7:1,0,6:1,1,6:1,2,6:2,0,5:2,1,5:2,2,5:3,0,4:3,1,4:3,2,4:4,0,3:4,1,3:4,2,3:5,0,2:5,1,2:5,2,2:6,0,1:6,1,1:6,2,1:7,0,0:7,1,0:7,2,0] SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P > qosmap_id
sai_get_qos_map_attribute $qosmap_id SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST 24
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP $qosmap_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP 1
dump $qosmap_id
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP 0
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
