#!/usr/bin/env python
#  saiShellHash.py
#
#*********************************************************************************/
#*  Copyright (c) 2021 Marvell. All rights reserved. The following file is       */
#*  subject to the limited use license agreement by and between Marvell and you, */
#*  your employer or other entity on behalf of whom you act. In the absence of   */
#*  such license agreement the following file is subject to Marvell's standard   */
#*  Limited Use License Agreement.                                               */
#*********************************************************************************/
#* @file saiShellHash.py
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
# The class object for saiShellHash operations
#/**********************************************************************************/

class saiShellHashObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*********************************************************/
    # command for sai_create_fine_grained_hash_field
    #/*********************************************************/
    def do_sai_create_fine_grained_hash_field(self, arg):

        ''' sai_create_fine_grained_hash_field '''

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
		    ret = sai_create_fine_grained_hash_field_default(int(args[0]))
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
		    ret = sai_create_fine_grained_hash_field_custom(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_create_hash
    #/*********************************************************/
    def do_sai_create_hash(self, arg):

        ''' sai_create_hash '''

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
		    ret = sai_create_hash_default(int(args[0]))
		    xpShellGlobals.cmdRetVal = long(ret)
        else:

	    varStr = ' switch_id, native_hash_field_list'
	    varList = varStr.replace(' ', '').split(',')
	    optArg = [ 'switch_id','native_hash_field_list' ]
	    args = ['0']*len(varList)
	    totNumArgs = len(varList)

	    argsList = re.split(' ',arg)
	    args = argsList
	    if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	    else:
		    listLen = 0
		    args[1] = args[1].replace(".",":").replace(",",":")
		    args[1] = args[1].strip(',').strip("'")
		    if args[1] != "[]":
			    postList = args[1].strip(']').strip('[').split(':')
			    listLen = len(postList)
		    native_hash_field_list_ptr = new_sai_s32_list_tp()
		    native_hash_field_list_ptr.count = listLen
		    native_hash_field_list_list_ptr = new_arr_sai_s32(listLen)
		    for ix in range(0, listLen, 1):
			    arr_sai_s32_setitem(native_hash_field_list_list_ptr, ix, int(postList[ix]))
		    native_hash_field_list_ptr.list = native_hash_field_list_list_ptr
		    ret = sai_create_hash_custom(int(args[0]),native_hash_field_list_ptr)
		    delete_sai_s32_list_tp(native_hash_field_list_ptr)
		    delete_arr_sai_s32(native_hash_field_list_list_ptr)
		    xpShellGlobals.cmdRetVal = long(ret)
    #/*********************************************************/
    # command for sai_set_fine_grained_hash_field_attribute
    #/*********************************************************/
    def do_sai_set_fine_grained_hash_field_attribute(self, arg):

	'''sai_set_fine_grained_hash_field_attribute: Enter[ fine_grained_hash_field_id, attr_id, attr_value ]'''

	varStr = 'fine_grained_hash_field_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'fine_grained_hash_field_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_FINE_GRAINED_HASH_FIELD_ATTR_IPV4_MASK'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			postList = postList[::-1]
			listLen = len(postList)
			attr_value_ptr.ip4 = 0
			for ix in range(listLen): 
				attr_value_ptr.ip4 = (attr_value_ptr.ip4*256)+int(postList[ix])
			ret = sai_set_fine_grained_hash_field_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_FINE_GRAINED_HASH_FIELD_ATTR_IPV6_MASK'):
			args[2] = args[2].replace(".",":").replace(",",":")
			postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
			listLen = len(postList)
			for ix in range(0, listLen, 1): 
				attr_value_ptr.ip6[ix] = int(postList[ix])
			ret = sai_set_fine_grained_hash_field_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_FINE_GRAINED_HASH_FIELD_ATTR_SEQUENCE_ID'):
			args[2]= int(args[2])
			attr_value_ptr.u32= args[2]
			ret = sai_set_fine_grained_hash_field_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		elif args[1] == eval('SAI_FINE_GRAINED_HASH_FIELD_ATTR_NATIVE_HASH_FIELD'):
			attr_value_ptr.s32= eval(args[2])
			ret = sai_set_fine_grained_hash_field_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)
    #/*********************************************************/
    # command for sai_set_hash_attribute
    #/*********************************************************/
    def do_sai_set_hash_attribute(self, arg):

	'''sai_set_hash_attribute: Enter[ hash_id, attr_id, attr_value ]'''

	varStr = 'hash_id, attr_id, attr_value'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hash_id','attr_id','attr_value' ]
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
		if args[1] == eval('SAI_HASH_ATTR_FINE_GRAINED_HASH_FIELD_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_hash_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		elif args[1] == eval('SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.s32list.count = listLen
			list_ptr = new_arr_sai_s32(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_s32_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.s32list.list = list_ptr
			ret = sai_set_hash_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_s32(list_ptr)
		elif args[1] == eval('SAI_HASH_ATTR_UDF_GROUP_LIST'):
			if args[2] != "[]":
				args[2] = args[2].replace(".",":").replace(",",":")
				postList = args[2].strip(',').strip("'").strip(']').strip('[').split(':')
				listLen = len(postList)
			attr_value_ptr.objlist.count = listLen
			list_ptr = new_arr_sai_object(listLen)
			for ix in range(0, listLen, 1):
				arr_sai_object_setitem(list_ptr, ix, int(postList[ix]))
			attr_value_ptr.objlist.list = list_ptr
			ret = sai_set_hash_attribute(args[0], args[1], attr_value_ptr, int(ipaddrflag),ipaddr_ptr)
			delete_arr_sai_object(list_ptr)
		else:
			print("Invalid attr_id "+str(args[1])+" recieved.")
		delete_sai_attribute_value_tp(attr_value_ptr)
		delete_sai_ip_address_tp(ipaddr_ptr)

    #/*********************************************************/
    # command for sai_get_fine_grained_hash_field_attribute
    #/*********************************************************/
    def do_sai_get_fine_grained_hash_field_attribute(self, arg):

	'''sai_get_fine_grained_hash_field_attribute: Enter[  fine_grained_hash_field_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' fine_grained_hash_field_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'fine_grained_hash_field_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_fine_grained_hash_field_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_get_hash_attribute
    #/*********************************************************/
    def do_sai_get_hash_attribute(self, arg):

	'''sai_get_hash_attribute: Enter[  hash_id, attr_id, count_if_attribute_is_list ]'''

	varStr = ' hash_id, attr_id, count_if_attribute_is_list'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hash_id','attr_id','count_if_attribute_is_list' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_get_hash_attribute(int(args[0]), eval(args[1]), int(args[2]),None)
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_fine_grained_hash_field
    #/*********************************************************/
    def do_sai_remove_fine_grained_hash_field(self, arg):

	'''sai_remove_fine_grained_hash_field: Enter[  fine_grained_hash_field_id ]'''

	varStr = ' fine_grained_hash_field_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'fine_grained_hash_field_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_fine_grained_hash_field(int(args[0]))
		xpShellGlobals.cmdRetVal = long(ret)

    #/*********************************************************/
    # command for sai_remove_hash
    #/*********************************************************/
    def do_sai_remove_hash(self, arg):

	'''sai_remove_hash: Enter[  hash_id ]'''

	varStr = ' hash_id'
	varList = varStr.replace(' ', '').split(',')
	optArg = [ 'hash_id' ]
	args = ['0']*len(varList)
	totNumArgs = len(varList)

	argsList = re.split(',| ',arg)
	args = argsList
	if (totNumArgs > 0 and argsList[0] == ''):
		print('Invalid input, Enter [ %s ]' % varStr)
	else:
		ret = sai_remove_hash(int(args[0]))
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