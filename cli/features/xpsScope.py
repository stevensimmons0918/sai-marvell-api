#!/usr/bin/env python
#  xpsScope.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

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

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsScope operations
#/**********************************************************************************/

class xpsScopeObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsScopeInit
    #/********************************************************************************/
    def do_scope_init(self, arg):
        '''
         xpsScopeInit: Enter [ initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ initType ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, initType=%d' % (args[0]))
            ret = xpsScopeInit(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsScopeDeInit
    #/********************************************************************************/
    def do_scope_de_init(self, arg):
        '''
         xpsScopeDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsScopeDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsScopeAddDevice
    #/********************************************************************************/
    def do_scope_add_device(self, arg):
        '''
         xpsScopeAddDevice: Enter [ devId,scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,scopeId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, scopeId=%d' % (args[0],args[1]))
            ret = xpsScopeAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsScopeRemoveDevice
    #/********************************************************************************/
    def do_scope_remove_device(self, arg):
        '''
         xpsScopeRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsScopeRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsScopeGetScopeId
    #/********************************************************************************/
    def do_scope_get_scope_id(self, arg):
        '''
         xpsScopeGetScopeId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            scopeId_Ptr_1 = new_xpsScope_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsScopeGetScopeId(args[0],scopeId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('scopeId = %d' % (xpsScope_tp_value(scopeId_Ptr_1)))
                pass
            delete_xpsScope_tp(scopeId_Ptr_1)
    #/********************************************************************************/
    # command for xpsScopeGetFirstDevice
    #/********************************************************************************/
    def do_scope_get_first_device(self, arg):
        '''
         xpsScopeGetFirstDevice: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            firstDevId_Ptr_1 = new_xpsDevice_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsScopeGetFirstDevice(args[0],firstDevId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('firstDevId = %d' % (xpsDevice_tp_value(firstDevId_Ptr_1)))
                pass
            delete_xpsDevice_tp(firstDevId_Ptr_1)
    #/********************************************************************************/
    # command for xpsScopeGetNextDevice
    #/********************************************************************************/
    def do_scope_get_next_device(self, arg):
        '''
         xpsScopeGetNextDevice: Enter [ scopeId,curDevId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,curDevId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            nextDevId_Ptr_2 = new_xpsDevice_tp()
            #print('Input Arguments are, scopeId=%d, curDevId=%d' % (args[0],args[1]))
            ret = xpsScopeGetNextDevice(args[0],args[1],nextDevId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nextDevId = %d' % (xpsDevice_tp_value(nextDevId_Ptr_2)))
                pass
            delete_xpsDevice_tp(nextDevId_Ptr_2)
    #/*****************************************************************************************************/
    # sub-commands to display tables
    #/*****************************************************************************************************/
    def do_display_tables(self, s):
        'Display Table commands'
        i = displayTableCmds()
        i.prompt = self.prompt[:-1]+':displayTables)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i
