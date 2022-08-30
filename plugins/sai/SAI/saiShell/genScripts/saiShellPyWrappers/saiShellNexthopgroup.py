#!/usr/bin/env python
#  saiShellNexthopgroup.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellNexthopgroup.py
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
# The class object for saiShellNexthopgroup operations
#/**********************************************************************************/

class saiShellNexthopgroupObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_next_hop_group
    #/*********************************************************/
    def do_sai_create_next_hop_group(self, arg):

        ''' sai_create_next_hop_group '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_next_hop_group_default(int(args[0]),eval(args[1]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, type'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_next_hop_group_custom(int(args[0]),eval(args[1]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_next_hop_group_member
    #/*********************************************************/
    def do_sai_create_next_hop_group_member(self, arg):

        ''' sai_create_next_hop_group_member '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, next_hop_id, next_hop_group_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','next_hop_id','next_hop_group_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_next_hop_group_member_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, next_hop_id, next_hop_group_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','next_hop_id','next_hop_group_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_next_hop_group_member_custom(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_next_hop_group_attribute
    #/*********************************************************/
    def do_sai_set_next_hop_group_attribute(self, arg):

	'''sai_set_next_hop_group_attribute: Enter[ next_hop_group_id, attr_id, attr_value ]'''

	varStr = 'next_hop_group_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_COUNTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_SET_SWITCHOVER'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_CONFIGURED_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_REAL_SIZE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_next_hop_group_member_attribute
    #/*********************************************************/
    def do_sai_set_next_hop_group_member_attribute(self, arg):

	'''sai_set_next_hop_group_member_attribute: Enter[ next_hop_group_member_id, attr_id, attr_value ]'''

	varStr = 'next_hop_group_member_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_member_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_OBSERVED_ROLE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_COUNTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_CONFIGURED_ROLE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_MONITORED_OBJECT'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_INDEX'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_NEXT_HOP_GROUP_MEMBER_ATTR_SEQUENCE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_next_hop_group_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_next_hop_group_attribute
    #/*********************************************************/
    def do_sai_get_next_hop_group_attribute(self, arg):

	'''sai_get_next_hop_group_attribute: Enter[  next_hop_group_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' next_hop_group_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_next_hop_group_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_next_hop_group_member_attribute
    #/*********************************************************/
    def do_sai_get_next_hop_group_member_attribute(self, arg):

	'''sai_get_next_hop_group_member_attribute: Enter[  next_hop_group_member_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' next_hop_group_member_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_member_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_next_hop_group_member_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_next_hop_group
    #/*********************************************************/
    def do_sai_remove_next_hop_group(self, arg):

	'''sai_remove_next_hop_group: Enter[  next_hop_group_id ]'''

	varStr = ' next_hop_group_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_next_hop_group(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_next_hop_group_member
    #/*********************************************************/
    def do_sai_remove_next_hop_group_member(self, arg):

	'''sai_remove_next_hop_group_member: Enter[  next_hop_group_member_id ]'''

	varStr = ' next_hop_group_member_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'next_hop_group_member_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_next_hop_group_member(int(args[0]))
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