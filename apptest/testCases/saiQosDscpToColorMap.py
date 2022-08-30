#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosDscpToColorMap',
    'description' : 'Verifies Dscp to color Qos map',
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
sai_create_qos_map 9288674231451648 [0,0:1,0:2,0:3,0:4,0:5,0:6,0:7,0:8,1:9,1:10,1:11,1:12,1:13,1:14,1:15,1:16,2:17,2:18,2:19,2:20,2:21,2:22,2:23,2:24,0:25,0:26,0:27,0:28,0:29,0:30,0:31,0:32,0:33,0:34,0:35,0:36,0:37,0:38,0:39,0:40,0:41,0:42,0:43,0:44,0:45,0:46,0:47,0:48,0:49,0:50,0:51,0:52,0:53,0:54,0:55,0:56,0:57,0:58,0:59,0:60,0:61,1:62,0:63,2] SAI_QOS_MAP_TYPE_DSCP_TO_COLOR > qosmap_id
sai_get_qos_map_attribute $qosmap_id SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST 64
back
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP $qosmap_id
sai_get_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP 1
dump $qosmap_id
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710656 SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP 0
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
