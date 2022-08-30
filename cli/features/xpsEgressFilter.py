#!/usr/bin/env python
#  xpsEgressFilter.py
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
# The class object for xpsEgressFilter operations
#/**********************************************************************************/

class xpsEgressFilterObj(Cmd):
    #/********************************************************************************/
    # command for xpsEgressFilterInit
    #/********************************************************************************/
    def do_egress_filter_init(self, arg):
        '''
         xpsEgressFilterInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsEgressFilterInit()
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterInitScope
    #/********************************************************************************/
    def do_egress_filter_init_scope(self, arg):
        '''
         xpsEgressFilterInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsEgressFilterInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterDeInit
    #/********************************************************************************/
    def do_egress_filter_de_init(self, arg):
        '''
         xpsEgressFilterDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsEgressFilterDeInit()
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterDeInitScope
    #/********************************************************************************/
    def do_egress_filter_de_init_scope(self, arg):
        '''
         xpsEgressFilterDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsEgressFilterDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterAddDevice
    #/********************************************************************************/
    def do_egress_filter_add_device(self, arg):
        '''
         xpsEgressFilterAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsEgressFilterAddDevice(args[0],args[1])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterRemoveDevice
    #/********************************************************************************/
    def do_egress_filter_remove_device(self, arg):
        '''
         xpsEgressFilterRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsEgressFilterRemoveDevice(args[0])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterCreate
    #/********************************************************************************/
    def do_egress_filter_create(self, arg):
        '''
         xpsEgressFilterCreate: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            egressFilterId_Ptr_0 = new_xpsEgressFilter_tp()
            #print('Input Arguments are' % ())
            ret = xpsEgressFilterCreate(egressFilterId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error Returned = %d' % (ret))
                err = 1
            if err == 0:
                print('egressFilterId = %d' % (xpsEgressFilter_tp_value(egressFilterId_Ptr_0)))
                pass
            delete_xpsEgressFilter_tp(egressFilterId_Ptr_0)

    #/********************************************************************************/
    # command for xpsEgressFilterCreateScope
    #/********************************************************************************/
    def do_egress_filter_create_scope(self, arg):
        '''
         xpsEgressFilterCreateScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            egressFilterId_Ptr_1 = new_xpsEgressFilter_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsEgressFilterCreateScope(args[0],egressFilterId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('egressFilterId = %d' % (xpsEgressFilter_tp_value(egressFilterId_Ptr_1)))
                pass
            delete_xpsEgressFilter_tp(egressFilterId_Ptr_1)

    #/********************************************************************************/
    # command for xpsEgressFilterDestroy
    #/********************************************************************************/
    def do_egress_filter_destroy(self, arg):
        '''
         xpsEgressFilterDestroy: Enter [ egressFilterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ egressFilterId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, egressFilterId=%d' % (args[0]))
            ret = xpsEgressFilterDestroy(args[0])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterDestroyScope
    #/********************************************************************************/
    def do_egress_filter_destroy_scope(self, arg):
        '''
         xpsEgressFilterDestroyScope: Enter [ scopeId,egressFilterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,egressFilterId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, egressFilterId=%d' % (args[0],args[1]))
            ret = xpsEgressFilterDestroyScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterSetAllPort
    #/********************************************************************************/
    def do_egress_filter_set_all_port(self, arg):
        '''
         xpsEgressFilterSetAllPort: Enter [ devId,egressFilterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,egressFilterId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, egressFilterId=%d' % (args[0],args[1]))
            ret = xpsEgressFilterSetAllPort(args[0],args[1])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterClearAllPort
    #/********************************************************************************/
    def do_egress_filter_clear_all_port(self, arg):
        '''
         xpsEgressFilterClearAllPort: Enter [ devId,egressFilterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,egressFilterId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, egressFilterId=%d' % (args[0],args[1]))
            ret = xpsEgressFilterClearAllPort(args[0],args[1])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterSetPort
    #/********************************************************************************/
    def do_egress_filter_set_port(self, arg):
        '''
         xpsEgressFilterSetPort: Enter [ devId,egressFilterId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,egressFilterId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, egressFilterId=%d, devPort=%d' % (args[0],args[1],args[2]))
            ret = xpsEgressFilterSetPort(args[0],args[1],args[2])
            if ret != 0:
                print('Error Returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsEgressFilterClearPort
    #/********************************************************************************/
    def do_egress_filter_clear_port(self, arg):
        '''
         xpsEgressFilterClearPort: Enter [ devId,egressFilterId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,egressFilterId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, egressFilterId=%d, devPort=%d' % (args[0],args[1],args[2]))
            ret = xpsEgressFilterClearPort(args[0],args[1],args[2])
            if ret != 0:
                print('Error Returned = %d' % (ret))
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

