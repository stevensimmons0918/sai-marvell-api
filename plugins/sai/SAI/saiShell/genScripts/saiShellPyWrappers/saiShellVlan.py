#!/usr/bin/env python
#  saiShellVlan.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellVlan.py
#

import sys
import time
import os
import re
import readline

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd
import xpShellGlobals
import saiShellGlobals
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for saiShellVlan operations
#/**********************************************************************************/

class saiShellVlanObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_vlan_member
    #/*********************************************************/
    def do_sai_create_vlan_member(self, arg):

        ''' sai_create_vlan_member '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, vlan_id, bridge_port_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','vlan_id','bridge_port_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_vlan_member_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, vlan_id, bridge_port_id, vlan_tagging_mode'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','vlan_id','bridge_port_id','vlan_tagging_mode' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_vlan_member_custom(int(args[0]),int(args[1]),int(args[2]),eval(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_vlan
    #/*********************************************************/
    def do_sai_create_vlan(self, arg):

        ''' sai_create_vlan '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, vlan_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','vlan_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_vlan_default(int(args[0]),int(args[1]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, ingress_acl, ipv4_mcast_lookup_key_type, custom_igmp_snooping_enable, egress_acl, vlan_id, unknown_multicast_flood_control_type, unknown_multicast_flood_group, broadcast_flood_group, broadcast_flood_control_type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','ingress_acl','ipv4_mcast_lookup_key_type','custom_igmp_snooping_enable','egress_acl','vlan_id','unknown_multicast_flood_control_type','unknown_multicast_flood_group','broadcast_flood_group','broadcast_flood_control_type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_vlan_custom(int(args[0]),int(args[1]),eval(args[2]),int(args[3]),int(args[4]),int(args[5]),eval(args[6]),int(args[7]),int(args[8]),eval(args[9]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_vlan_member_attribute
    #/*********************************************************/
    def do_sai_set_vlan_member_attribute(self, arg):

	'''sai_set_vlan_member_attribute: Enter[ vlan_member_id, attr_id, attr_value ]'''

	varStr = 'vlan_member_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_member_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_VLAN_MEMBER_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_vlan_attribute
    #/*********************************************************/
    def do_sai_set_vlan_attribute(self, arg):

	'''sai_set_vlan_attribute: Enter[ vlan_id, attr_id, attr_value ]'''

	varStr = 'vlan_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id','attr_id','attr_value' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		args[0] = int(args[0])
		args[1]=eval(args[1])

		attr_value_ptr = new_sai_attribute_value_tp()
		ipaddr_ptr = new_sai_ip_address_tp()
		ipaddrflag = 0
		listLen = 0
		if args[1] == eval('SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_INGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_STP_INSTANCE'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_TAM_OBJECT'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_CUSTOM_IGMP_SNOOPING_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_EGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_LEARN_DISABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_MEMBER_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_META_DATA'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_vlan_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_vlan_member_attribute
    #/*********************************************************/
    def do_sai_get_vlan_member_attribute(self, arg):

	'''sai_get_vlan_member_attribute: Enter[  vlan_member_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' vlan_member_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_member_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_vlan_member_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_vlan_attribute
    #/*********************************************************/
    def do_sai_get_vlan_attribute(self, arg):

	'''sai_get_vlan_attribute: Enter[  vlan_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' vlan_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_vlan_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_vlan_stats
    #/*********************************************************/
    def do_sai_get_vlan_stats(self, arg):

	'''sai_get_vlan_stats: Enter[  vlan_id, number_of_counters ]'''

	varStr = ' vlan_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 1)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_vlan_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 1):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)
    #/*********************************************************/
    # command for sai_clear_vlan_stats
    #/*********************************************************/
    def do_sai_clear_vlan_stats(self, arg):

	'''sai_clear_vlan_stats: Enter[  vlan_id, number_of_counters ]'''

	varStr = ' vlan_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id','number_of_counters' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 1)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 1):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_clear_vlan_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_vlan_stats_ext
    #/*********************************************************/
    def do_sai_get_vlan_stats_ext(self, arg):

	'''sai_get_vlan_stats_ext: Enter[  vlan_id, number_of_counters, mode ]'''

	varStr = ' vlan_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id','number_of_counters','mode' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(r'[;,\s]\s*',arg)
	args = argsList
	if ((totNumArgs > 0 and argsList[0] == '') or (totNumArgs <= 1)):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		counters_list = new_arrUint64(len(argsList) - 2)
		arr_stat_ptr = new_arr_sai_stat_id_tp(len(argsList) - 2)
		argsLen = len(argsList)
		for ix in range(0, argsLen - 2):
			arr_sai_stat_id_tp_setitem(arr_stat_ptr, ix, eval(args[ix + 1]))
		ret = sai_get_vlan_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			resultStr = ''
			for ix in range(0, argsLen - 2):
				temp = arrUint64_getitem(counters_list, ix)
				resultStr = resultStr + ' ' + str(args[ix + 1]) + ' : ' + str(temp) + '\n'
			print('%s' % resultStr)
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
		delete_arrUint64(counters_list)

    #/*********************************************************/
    # command for sai_remove_vlan_member
    #/*********************************************************/
    def do_sai_remove_vlan_member(self, arg):

	'''sai_remove_vlan_member: Enter[  vlan_member_id ]'''

	varStr = ' vlan_member_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_member_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_vlan_member(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_vlan
    #/*********************************************************/
    def do_sai_remove_vlan(self, arg):

	'''sai_remove_vlan: Enter[  vlan_id ]'''

	varStr = ' vlan_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'vlan_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_vlan(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for alter_create_mode
    #/*********************************************************/
    def do_alter_create_mode(self, arg):

        '''alter_create_mode: Enter [ 'create_mode' ] '''

        varStr = 'create_mode'
        varList = varStr.replace(' ', '').split(',')
        optArg = [ 'create_mode' ]
        args = ['0']*len(varList)
        totNumArgs = len(varList)

        argsList = re.split(',| ',arg)
        args = argsList
        if (totNumArgs > 0 and argsList[0] == ''):
            print('Invalid input, Enter [ %s ]' % varStr)
        else:
            xpShellGlobals.cmdRetVal = long(int(args[0]))
            saiShellGlobals.createMode = int(args[0])