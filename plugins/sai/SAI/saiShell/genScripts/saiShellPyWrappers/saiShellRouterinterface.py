#!/usr/bin/env python
#  saiShellRouterinterface.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellRouterinterface.py
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
# The class object for saiShellRouterinterface operations
#/**********************************************************************************/

class saiShellRouterinterfaceObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_router_interface
    #/*********************************************************/
    def do_sai_create_router_interface(self, arg):

        ''' sai_create_router_interface '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, vlan_id, type, port_id, virtual_router_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','vlan_id','type','port_id','virtual_router_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_router_interface_default(int(args[0]),int(args[1]),eval(args[2]),int(args[3]),int(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, src_mac_address, admin_v4_state, v4_mcast_enable, vlan_id, ingress_acl, type, admin_v6_state, egress_acl, neighbor_miss_packet_action, mtu, v6_mcast_enable, port_id, virtual_router_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','src_mac_address','admin_v4_state','v4_mcast_enable','vlan_id','ingress_acl','type','admin_v6_state','egress_acl','neighbor_miss_packet_action','mtu','v6_mcast_enable','port_id','virtual_router_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[1] = args[1].replace(".",":").replace(",",":")
		    postList = args[1].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    src_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_router_interface_custom(int(args[0]),src_mac_address_ptr,int(args[2]),int(args[3]),int(args[4]),int(args[5]),eval(args[6]),int(args[7]),int(args[8]),eval(args[9]),int(args[10]),int(args[11]),int(args[12]),int(args[13]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_router_interface_attribute
    #/*********************************************************/
    def do_sai_set_router_interface_attribute(self, arg):

	'''sai_set_router_interface_attribute: Enter[ router_interface_id, attr_id, attr_value ]'''

	varStr = 'router_interface_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_BRIDGE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_NAT_ZONE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_DISABLE_DECREMENT_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_INNER_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_OUTER_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_MTU'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_LOOPBACK_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_ROUTER_INTERFACE_ATTR_IS_VIRTUAL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_router_interface_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_router_interface_attribute
    #/*********************************************************/
    def do_sai_get_router_interface_attribute(self, arg):

	'''sai_get_router_interface_attribute: Enter[  router_interface_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' router_interface_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_router_interface_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_get_router_interface_stats
    #/*********************************************************/
    def do_sai_get_router_interface_stats(self, arg):

	'''sai_get_router_interface_stats: Enter[  router_interface_id, number_of_counters ]'''

	varStr = ' router_interface_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id','number_of_counters' ]
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
		ret = sai_get_router_interface_stats(int(args[0]),(argsLen - 1), arr_stat_ptr, counters_list)
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
    # command for sai_clear_router_interface_stats
    #/*********************************************************/
    def do_sai_clear_router_interface_stats(self, arg):

	'''sai_clear_router_interface_stats: Enter[  router_interface_id, number_of_counters ]'''

	varStr = ' router_interface_id, number_of_counters'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id','number_of_counters' ]
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
		ret = sai_clear_router_interface_stats(int(args[0]), (argsLen - 1), arr_stat_ptr)
		if(ret != 0):
			print('Failled with RC: %d' % ret)
		else:
			print('Success')
		delete_arr_sai_stat_id_tp(arr_stat_ptr)
    #/*********************************************************/
    # command for sai_get_router_interface_stats_ext
    #/*********************************************************/
    def do_sai_get_router_interface_stats_ext(self, arg):

	'''sai_get_router_interface_stats_ext: Enter[  router_interface_id, number_of_counters, mode ]'''

	varStr = ' router_interface_id, number_of_counters, mode'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id','number_of_counters','mode' ]
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
		ret = sai_get_router_interface_stats_ext(int(args[0]),(argsLen - 2), arr_stat_ptr, (eval(args[argsLen - 1])), counters_list)
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
    # command for sai_remove_router_interface
    #/*********************************************************/
    def do_sai_remove_router_interface(self, arg):

	'''sai_remove_router_interface: Enter[  router_interface_id ]'''

	varStr = ' router_interface_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'router_interface_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_router_interface(int(args[0]))
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