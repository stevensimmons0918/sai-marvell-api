#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosEgressBufferProfile',
    'description' : 'Verifies egress buffer profile create,set,get & remove',
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
buffer
sai_create_buffer_pool 9288674231451648 10000000 0 1 > bp_static_egress
sai_create_buffer_pool 9288674231451648 10000000 1 1 > bp_dynamic_egress
sai_create_buffer_profile 9288674231451648 0 0 500000 0 $bp_static_egress > buffer_prof_static
sai_create_buffer_profile 9288674231451648 -4 0 30000 1 $bp_dynamic_egress > buffer_prof_dynamic
sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE 1
sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH 1
back
queue
sai_set_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID $buffer_prof_static
sai_set_queue_attribute 5910974510923780 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID $buffer_prof_dynamic
sai_get_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID 1
sai_get_queue_attribute 5910974510923780 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID 1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
queue
sai_set_queue_attribute 5910974510923780 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID 0
sai_set_queue_attribute 5910974510923783 SAI_QUEUE_ATTR_BUFFER_PROFILE_ID 0
back
buffer
sai_remove_buffer_profile $buffer_prof_static
sai_remove_buffer_profile $buffer_prof_dynamic
sai_remove_buffer_pool $bp_static_egress
sai_remove_buffer_pool $bp_dynamic_egress
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'7036874417766401'
}
