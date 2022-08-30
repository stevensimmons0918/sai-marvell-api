#!/usr/bin/env python
#  xpsLink.py
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
# The class object for xpsLink operations
#/**********************************************************************************/

class xpsLinkObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsLinkInit
    #/********************************************************************************/
    def do_link_init(self, arg):
        '''
         xpsLinkInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsLinkInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLinkInitScope
    #/********************************************************************************/
    def do_link_init_scope(self, arg):
        '''
         xpsLinkInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLinkInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLinkDeInit
    #/********************************************************************************/
    def do_link_de_init(self, arg):
        '''
         xpsLinkDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsLinkDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLinkDeInitScope
    #/********************************************************************************/
    def do_link_de_init_scope(self, arg):
        '''
         xpsLinkDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLinkDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLinkAddDevice
    #/********************************************************************************/
    def do_link_add_device(self, arg):
        '''
         xpsLinkAddDevice: Enter [ devId,initType]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsLinkAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLinkRemoveDevice
    #/********************************************************************************/
    def do_link_remove_device(self, arg):
        '''
         xpsLinkRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLinkRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
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

    #/********************************************************************************/
    # command for xpsLinkGetFaultLiveStatus
    #/********************************************************************************/
    def do_get_fault_live_status(self, arg):
        '''
         xpsLinkGetFaultLiveStatus: Enter [ devId,portNum/startPort-endPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum/startPort-endPort ]')
        else:
            args[0] = int(args[0])
            if '-' in args[1]:
                startIdx = int(args[1].split('-')[0].strip())
                endIdx = int(args[1].split('-')[1].strip())
                if startIdx > endIdx:
                    print('Improper Range')
                    return
            else:
                args[1] = int(args[1])
                startIdx = args[1]
                endIdx = args[1]
            enable_Ptr_2 = new_xpMacRxFaultp()
            #print('Input Arguments are, devId=%d, portNum/startPort-endPort=%d' % (args[0],args[1]))
            valid_Ptr = new_uint8_tp()
            for portNum in range(startIdx,endIdx+1):
                xpLinkManagerIsPortNumValid(args[0],portNum,valid_Ptr)
                if((uint8_tp_value(valid_Ptr))):
                    if(xpLinkManagerIsPortInited(args[0],portNum) == 0):
                        ret = xpsLinkGetFaultLiveStatus(args[0],portNum,enable_Ptr_2)
                        err = 0
                        if ret != 0:
                            print('Return Value = %d for portNum = %d' % (ret,portNum))
                            err = 1
                        if err == 0:
                            print('============================')
                            print('Port %d:' % (portNum))
                            print('============================')
                            if(xpMacRxFaultp_value(enable_Ptr_2) == 0):
                                print('fault = LOCAL')
                            if(xpMacRxFaultp_value(enable_Ptr_2) == 1):
                                print('fault = REMOTE')
                            if(xpMacRxFaultp_value(enable_Ptr_2) == 2):
                                print('fault = OK')
                    else:
                        print('============================')
                        print('Port %d:' % (portNum))
                        print('============================')
                        print('portNum %d is not initialized' % (portNum))
                        print('')
                else:
                    pass
            delete_uint8_tp(enable_Ptr_2)

