#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiQosIngressBufferProfile',
    'description' : 'Verifies Ingress buffer profile create,set,get & remove',
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
sai_create_buffer_pool 9288674231451648 10000000 0 0 > bp_static_ingress
sai_create_buffer_pool 9288674231451648 10000000 1 0 > bp_dynamic_ingress
sai_create_buffer_profile 9288674231451648 0 0 1000000 0 $bp_static_ingress > buffer_prof_static
sai_create_buffer_profile 9288674231451648 -1 0 2000000 1 $bp_dynamic_ingress > buffer_prof_dynamic
sai_set_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_XOFF_TH 4096
sai_set_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_XON_TH 100000
sai_set_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_XOFF_TH 4096
sai_set_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_XON_TH 100000

sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE 1
sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH 1
sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_XOFF_TH 1
sai_get_buffer_profile_attribute $buffer_prof_static SAI_BUFFER_PROFILE_ATTR_XON_TH 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_XOFF_TH 1
sai_get_buffer_profile_attribute $buffer_prof_dynamic SAI_BUFFER_PROFILE_ATTR_XON_TH 1

sai_set_ingress_priority_group_attribute 7318349394477056 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE $buffer_prof_static
sai_get_ingress_priority_group_attribute 7318349394477056 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE 1
sai_set_ingress_priority_group_attribute 7318349394477057 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE $buffer_prof_dynamic
sai_get_ingress_priority_group_attribute 7318349394477057 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE 1
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
buffer
sai_set_ingress_priority_group_attribute 7318349394477056 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE 0
sai_set_ingress_priority_group_attribute 7318349394477057 SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE 0
sai_remove_buffer_profile $buffer_prof_static
sai_remove_buffer_profile $buffer_prof_dynamic
sai_remove_buffer_pool $bp_static_ingress
sai_remove_buffer_pool $bp_dynamic_ingress
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'7036874417766401'
}
