#!/usr/bin/env python
#  saiShellLag.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellLag.py
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
# The class object for saiShellLag operations
#/**********************************************************************************/

class saiShellLagObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_lag
    #/*********************************************************/
    def do_sai_create_lag(self, arg):

        ''' sai_create_lag '''

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
		    ret = sai_create_lag_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, egress_acl, ingress_acl, port_vlan_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','egress_acl','ingress_acl','port_vlan_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_lag_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_lag_member
    #/*********************************************************/
    def do_sai_create_lag_member(self, arg):

        ''' sai_create_lag_member '''

        if saiShellGlobals.createMode == 0:

	    varStr = ' switch_id, port_id, lag_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','port_id','lag_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_lag_member_default(int(args[0]),int(args[1]),int(args[2]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, egress_disable, ingress_disable, port_id, lag_id'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','egress_disable','ingress_disable','port_id','lag_id' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    ret = sai_create_lag_member_custom(int(args[0]),int(args[1]),int(args[2]),int(args[3]),int(args[4]))
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_lag_attribute
    #/*********************************************************/
    def do_sai_set_lag_attribute(self, arg):

	'''sai_set_lag_attribute: Enter[ lag_id, attr_id, attr_value ]'''

	varStr = 'lag_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_LAG_ATTR_EGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_INGRESS_ACL'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_PORT_VLAN_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_PORT_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_DROP_TAGGED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_TPID'):
			args[2]= int(args[2])
			attr_value_ptr.u16= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_LABEL'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList[0])
			for ix in range(0, listLen, 1):
				attr_value_ptr.chardata[ix] = postList[0][ix]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_DROP_UNTAGGED'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_ATTR_SYSTEM_PORT_AGGREGATE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_lag_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_lag_member_attribute
    #/*********************************************************/
    def do_sai_set_lag_member_attribute(self, arg):

	'''sai_set_lag_member_attribute: Enter[ lag_member_id, attr_id, attr_value ]'''

	varStr = 'lag_member_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_member_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_lag_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE'):
			args[2]= int(args[2])
			attr_value_ptr.u8= args[2]
			ret = sai_set_lag_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_MEMBER_ATTR_PORT_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_lag_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_LAG_MEMBER_ATTR_LAG_ID'):
			args[2]= int(args[2])
			attr_value_ptr.oid= args[2]
			ret = sai_set_lag_member_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_lag_attribute
    #/*********************************************************/
    def do_sai_get_lag_attribute(self, arg):

	'''sai_get_lag_attribute: Enter[  lag_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' lag_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_lag_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_lag_member_attribute
    #/*********************************************************/
    def do_sai_get_lag_member_attribute(self, arg):

	'''sai_get_lag_member_attribute: Enter[  lag_member_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' lag_member_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_member_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_lag_member_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_lag
    #/*********************************************************/
    def do_sai_remove_lag(self, arg):

	'''sai_remove_lag: Enter[  lag_id ]'''

	varStr = ' lag_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_lag(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_lag_member
    #/*********************************************************/
    def do_sai_remove_lag_member(self, arg):

	'''sai_remove_lag_member: Enter[  lag_member_id ]'''

	varStr = ' lag_member_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'lag_member_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_lag_member(int(args[0]))
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