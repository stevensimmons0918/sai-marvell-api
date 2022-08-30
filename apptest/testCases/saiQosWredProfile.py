#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosWredProfile',
    'description' : 'Verifies wred profile create,set,get & remove',
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
wred
sai_create_wred 9288674231451648 > wred_id
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_ENABLE 1
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_ENABLE 1
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_RED_ENABLE 1
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_ECN_MARK_MODE SAI_ECN_MARK_MODE_ALL
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_MIN_THRESHOLD 5000
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD 3000
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_RED_MIN_THRESHOLD 1000
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_DROP_PROBABILITY 50
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_RED_DROP_PROBABILITY 70
sai_set_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY 90

sai_get_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_ENABLE 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_MIN_THRESHOLD 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_GREEN_DROP_PROBABILITY 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_ENABLE 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_RED_ENABLE 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_RED_MIN_THRESHOLD 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_RED_DROP_PROBABILITY 1
sai_get_wred_attribute $wred_id SAI_WRED_ATTR_ECN_MARK_MODE 1
back
queue
sai_set_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_WRED_PROFILE_ID $wred_id
sai_get_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_WRED_PROFILE_ID 1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
queue
sai_set_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_WRED_PROFILE_ID 0
back
wred
sai_remove_wred $wred_id
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'5348024557502464'
}
