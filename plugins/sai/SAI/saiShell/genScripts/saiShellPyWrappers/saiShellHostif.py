#!/usr/bin/env python
#  saiShellHostif.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellHostif.py
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
# The class object for saiShellHostif operations
#/**********************************************************************************/

class saiShellHostifObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_hostif
    #/*********************************************************/
    def do_sai_create_hostif(self, arg):

        ''' sai_create_hostif '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, type, name, obj_id, genetlink_mcgrp_name'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type','name','obj_id','genetlink_mcgrp_name' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[2] = args[2].replace(".",":").replace(",",":")
		    postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList[0])
		    name_ptr = new_shell_stringp()
		    for ix in range(0, listLen, 1):
			    name_ptr.name[ix] = postList[0][ix]
		    name_ptr.name[listLen] = '\0'
		    args[4] = args[4].replace(".",":").replace(",",":")
		    postList = args[4].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList[0])
		    genetlink_mcgrp_name_ptr = new_shell_stringp()
		    for ix in range(0, listLen, 1):
			    genetlink_mcgrp_name_ptr.name[ix] = postList[0][ix]
		    genetlink_mcgrp_name_ptr.name[listLen] = '\0'
		    ret = sai_create_hostif_default(int(args[0]),eval(args[1]),name_ptr,int(args[3]),genetlink_mcgrp_name_ptr)
		    delete_shell_stringp(name_ptr)
		    delete_shell_stringp(genetlink_mcgrp_name_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, vlan_tag, type, name, obj_id, oper_status, genetlink_mcgrp_name'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','vlan_tag','type','name','obj_id','oper_status','genetlink_mcgrp_name' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    args[3] = args[3].replace(".",":").replace(",",":")
		    postList = args[3].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList[0])
		    name_ptr = new_shell_stringp()
		    for ix in range(0, listLen, 1):
			    name_ptr.name[ix] = postList[0][ix]
		    name_ptr.name[listLen] = '\0'
		    args[6] = args[6].replace(".",":").replace(",",":")
		    postList = args[6].strip(',').strip("'").strip(']').strip('[').split(':')
		    listLen = len(postList[0])
		    genetlink_mcgrp_name_ptr = new_shell_stringp()
		    for ix in range(0, listLen, 1):
			    genetlink_mcgrp_name_ptr.name[ix] = postList[0][ix]
		    genetlink_mcgrp_name_ptr.name[listLen] = '\0'
		    ret = sai_create_hostif_custom(int(args[0]),eval(args[1]),eval(args[2]),name_ptr,int(args[4]),int(args[5]),genetlink_mcgrp_name_ptr)
		    delete_shell_stringp(name_ptr)
		    delete_shell_stringp(genetlink_mcgrp_name_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_hostif_trap_group
    #/*********************************************************/
    def do_sai_create_hostif_trap_group(self, arg):

        ''' sai_create_hostif_trap_group '''

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
		    ret = sai_create_hostif_trap_group_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, queue, policer, admin_state'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','queue','policer','admin_state' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_hostif_trap_group_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_hostif_user_defined_trap
    #/*********************************************************/
    def do_sai_create_hostif_user_defined_trap(self, arg):

        ''' sai_create_hostif_user_defined_trap '''

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
		    ret = sai_create_hostif_user_defined_trap_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

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
		    ret = sai_create_hostif_user_defined_trap_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_hostif_trap
    #/*********************************************************/
    def do_sai_create_hostif_trap(self, arg):

        ''' sai_create_hostif_trap '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, trap_type, packet_action'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','trap_type','packet_action' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_hostif_trap_default(int(args[0]),eval(args[1]),eval(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, trap_type, exclude_port_list, packet_action, trap_group'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','trap_type','exclude_port_list','packet_action','trap_group' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[2] = args[2].replace(".",":").replace(",",":")
		    args[2] = args[2].strip(',').strip("'")
		    if args[2] != "[]":
			    postList = args[2].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    exclude_port_list_ptr = new_sai_object_list_tp()
		    exclude_port_list_ptr.count = listLen
		    exclude_port_list_list_ptr = new_arr_sai_object(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_object_setitem(exclude_port_list_list_ptr, ix, int(postList[ix]))
		    exclude_port_list_ptr.list = exclude_port_list_list_ptr
		    ret = sai_create_hostif_trap_custom(int(args[0]),eval(args[1]),exclude_port_list_ptr,eval(args[3]),int(args[4]))
		    delete_sai_object_list_tp(exclude_port_list_ptr)
		    delete_arr_sai_object(exclude_port_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_hostif_table_entry
    #/*********************************************************/
    def do_sai_create_hostif_table_entry(self, arg):

        ''' sai_create_hostif_table_entry '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, type, trap_id, channel_type, host_if'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type','trap_id','channel_type','host_if' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_hostif_table_entry_default(int(args[0]),eval(args[1]),int(args[2]),eval(args[3]),int(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, type, trap_id, channel_type, host_if'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','type','trap_id','channel_type','host_if' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_hostif_table_entry_custom(int(args[0]),eval(args[1]),int(args[2]),eval(args[3]),int(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_hostif_attribute
    #/*********************************************************/
    def do_sai_set_hostif_attribute(self, arg):

	'''sai_set_hostif_attribute: Enter[ hostif_id, attr_id, attr_value ]'''

	varStr = 'hostif_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HOSTIF_ATTR_VLAN_TAG'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_NAME'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList[0])
			for ix in range(0, listLen, 1):
				attr_value_ptr.chardata[ix] = postList[0][ix]
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_OBJ_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_QUEUE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_OPER_STATUS'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_ATTR_GENETLINK_MCGRP_NAME'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList[0])
			for ix in range(0, listLen, 1):
				attr_value_ptr.chardata[ix] = postList[0][ix]
			ret = sai_set_hostif_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_hostif_trap_group_attribute
    #/*********************************************************/
    def do_sai_set_hostif_trap_group_attribute(self, arg):

	'''sai_set_hostif_trap_group_attribute: Enter[ hostif_trap_group_id, attr_id, attr_value ]'''

	varStr = 'hostif_trap_group_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_group_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_hostif_trap_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_trap_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_hostif_trap_group_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_hostif_user_defined_trap_attribute
    #/*********************************************************/
    def do_sai_set_hostif_user_defined_trap_attribute(self, arg):

	'''sai_set_hostif_user_defined_trap_attribute: Enter[ hostif_user_defined_trap_id, attr_id, attr_value ]'''

	varStr = 'hostif_user_defined_trap_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_user_defined_trap_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_user_defined_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_hostif_user_defined_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_user_defined_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_hostif_trap_attribute
    #/*********************************************************/
    def do_sai_set_hostif_trap_attribute(self, arg):

	'''sai_set_hostif_trap_attribute: Enter[ hostif_trap_id, attr_id, attr_value ]'''

	varStr = 'hostif_trap_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_COUNTER_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TRAP_ATTR_MIRROR_SESSION'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_hostif_trap_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_hostif_table_entry_attribute
    #/*********************************************************/
    def do_sai_set_hostif_table_entry_attribute(self, arg):

	'''sai_set_hostif_table_entry_attribute: Enter[ hostif_table_entry_id, attr_id, attr_value ]'''

	varStr = 'hostif_table_entry_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_table_entry_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_hostif_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_hostif_table_entry_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_hostif_attribute
    #/*********************************************************/
    def do_sai_get_hostif_attribute(self, arg):

	'''sai_get_hostif_attribute: Enter[  hostif_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hostif_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hostif_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_hostif_trap_group_attribute
    #/*********************************************************/
    def do_sai_get_hostif_trap_group_attribute(self, arg):

	'''sai_get_hostif_trap_group_attribute: Enter[  hostif_trap_group_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hostif_trap_group_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_group_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hostif_trap_group_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_hostif_user_defined_trap_attribute
    #/*********************************************************/
    def do_sai_get_hostif_user_defined_trap_attribute(self, arg):

	'''sai_get_hostif_user_defined_trap_attribute: Enter[  hostif_user_defined_trap_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hostif_user_defined_trap_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_user_defined_trap_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hostif_user_defined_trap_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_hostif_trap_attribute
    #/*********************************************************/
    def do_sai_get_hostif_trap_attribute(self, arg):

	'''sai_get_hostif_trap_attribute: Enter[  hostif_trap_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hostif_trap_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hostif_trap_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_hostif_table_entry_attribute
    #/*********************************************************/
    def do_sai_get_hostif_table_entry_attribute(self, arg):

	'''sai_get_hostif_table_entry_attribute: Enter[  hostif_table_entry_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hostif_table_entry_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_table_entry_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hostif_table_entry_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hostif
    #/*********************************************************/
    def do_sai_remove_hostif(self, arg):

	'''sai_remove_hostif: Enter[  hostif_id ]'''

	varStr = ' hostif_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hostif(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hostif_trap_group
    #/*********************************************************/
    def do_sai_remove_hostif_trap_group(self, arg):

	'''sai_remove_hostif_trap_group: Enter[  hostif_trap_group_id ]'''

	varStr = ' hostif_trap_group_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_group_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hostif_trap_group(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hostif_user_defined_trap
    #/*********************************************************/
    def do_sai_remove_hostif_user_defined_trap(self, arg):

	'''sai_remove_hostif_user_defined_trap: Enter[  hostif_user_defined_trap_id ]'''

	varStr = ' hostif_user_defined_trap_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_user_defined_trap_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hostif_user_defined_trap(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hostif_trap
    #/*********************************************************/
    def do_sai_remove_hostif_trap(self, arg):

	'''sai_remove_hostif_trap: Enter[  hostif_trap_id ]'''

	varStr = ' hostif_trap_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_trap_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hostif_trap(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hostif_table_entry
    #/*********************************************************/
    def do_sai_remove_hostif_table_entry(self, arg):

	'''sai_remove_hostif_table_entry: Enter[  hostif_table_entry_id ]'''

	varStr = ' hostif_table_entry_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hostif_table_entry_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hostif_table_entry(int(args[0]))
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