#!/usr/bin/env python
#  saiShellVirtualrouter.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellVirtualrouter.py
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
# The class object for saiShellVirtualrouter operations
#/**********************************************************************************/

class saiShellVirtualrouterObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_virtual_router
    #/*********************************************************/
    def do_sai_create_virtual_router(self, arg):

        ''' sai_create_virtual_router '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_virtual_router_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, admin_v4_state, unknown_l3_multicast_packet_action, admin_v6_state, src_mac_address'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','admin_v4_state','unknown_l3_multicast_packet_action','admin_v6_state','src_mac_address' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList)
		    src_mac_address_ptr = new_shell_macp()
		    for ix in range(0, listLen, 1):
			    src_mac_address_ptr.mac[ix] =  int(postList[ix], 16)
		    ret = sai_create_virtual_router_custom(int(args[0]),int(args[1]),eval(args[2]),int(args[3]),src_mac_address_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_virtual_router_attribute
    #/*********************************************************/
    def do_sai_set_virtual_router_attribute(self, arg):

	'''sai_set_virtual_router_attribute: Enter[ virtual_router_id, attr_id, attr_value ]'''

	varStr = 'virtual_router_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'virtual_router_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_LABEL'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList[0])
			for ix in range(0, listLen, 1):
				attr_value_ptr.chardata[ix] = postList[0][ix]
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.mac[ix] = int(postList[ix], 16)
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_virtual_router_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_virtual_router_attribute
    #/*********************************************************/
    def do_sai_get_virtual_router_attribute(self, arg):

	'''sai_get_virtual_router_attribute: Enter[  virtual_router_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' virtual_router_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'virtual_router_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_virtual_router_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_virtual_router
    #/*********************************************************/
    def do_sai_remove_virtual_router(self, arg):

	'''sai_remove_virtual_router: Enter[  virtual_router_id ]'''

	varStr = ' virtual_router_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'virtual_router_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_virtual_router(int(args[0]))
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