#!/usr/bin/env python
#  saiShellNexthop.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellNexthop.py
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
# The class object for saiShellNexthop operations
#/**********************************************************************************/

class saiShellNexthopObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_next_hop
    #/*********************************************************/
    def do_sai_create_next_hop(self, arg):

        ''' sai_create_next_hop '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, tunnel_id, router_interface_id, type, ip_addr_family, ip'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','tunnel_id','router_interface_id','type','ip_addr_family','ip' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[5] = args[5].replace(".",":").replace(",",":")
		    postList = args[5].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    ip_ptr = new_sai_ip_address_tp()
		    ip_ptr.addr_family = eval(args[4])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    ret = sai_create_next_hop_default(int(args[0]),int(args[1]),int(args[2]),eval(args[3]),ip_ptr)
		    delete_sai_ip_address_tp(ip_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, tunnel_id, router_interface_id, type, tunnel_vni, ip_addr_family, ip, tunnel_mac'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','tunnel_id','router_interface_id','type','tunnel_vni','ip_addr_family','ip','tunnel_mac' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[6] = args[6].replace(".",":").replace(",",":")
		    postList = args[6].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    ip_ptr = new_sai_ip_address_tp()
		    ip_ptr.addr_family = eval(args[5])
		    base = 10
		    if listLen > 4:
			    base = 16
		    for ix in range(0, listLen, 1):
			    ip_ptr.addr.ip6[ix] = int(postList[ix], base)
		    args[7] = args[7].replace(".",":").replace(",",":")
		    postList = args[7].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    tunnel_mac_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    tunnel_mac_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_next_hop_custom(int(args[0]),int(args[1]),int(args[2]),eval(args[3]),int(args[4]),ip_ptr,tunnel_mac_ptr)
		    delete_sai_ip_address_tp(ip_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_next_hop_attribute
    #/*********************************************************/
    def do_sai_set_next_hop_attribute(self, arg):

	'''sai_set_next_hop_attribute: Enter[ next_hop_id, attr_id, attr_value ]'''

	varStr = 'next_hop_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_NEXT_HOP_ATTR_OUTSEG_TTL_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_OUTSEG_EXP_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_TUNNEL_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_COUNTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_QOS_TC_AND_COLOR_TO_MPLS_EXP_MAP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_LABELSTACK'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.u32list.count = listLen
			list_ptr = new_arr_sai_u32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_u32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.u32list.list = list_ptr
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_u32(list_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_DISABLE_DECREMENT_TTL'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_TUNNEL_VNI'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_IP'):
			ipaddrflag = 1
			args[3] = args[3].replace(".",":").replace(",",":")
			postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			ipaddr_ptr.addr_family = eval(args[2])
			base = 10
			if listLen > 4:
				base = 16
			for ix in range(0, listLen, 1):
				ipaddr_ptr.addr.ip6[ix] = int(postList[ix], base)
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_SEGMENTROUTE_SIDLIST_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_OUTSEG_TTL_MODE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_TUNNEL_MAC'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_OUTSEG_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_OUTSEG_EXP_VALUE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_SEGMENTROUTE_ENDPOINT_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_ATTR_SEGMENTROUTE_ENDPOINT_POP_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_next_hop_attribute
    #/*********************************************************/
    def do_sai_get_next_hop_attribute(self, arg):

	'''sai_get_next_hop_attribute: Enter[  next_hop_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' next_hop_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_next_hop_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_next_hop
    #/*********************************************************/
    def do_sai_remove_next_hop(self, arg):

	'''sai_remove_next_hop: Enter[  next_hop_id ]'''

	varStr = ' next_hop_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_next_hop(int(args[0]))
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