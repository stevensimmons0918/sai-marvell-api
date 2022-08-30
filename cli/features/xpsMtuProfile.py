#!/usr/bin/env python
#  xpsMtuProfile.py
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
# The class object for xpsMtuProfile operations
#/**********************************************************************************/

class xpsMtuProfileObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'


    #/********************************************************************************/
    # command for xpsMtuInitScope
    #/********************************************************************************/
    def do_mtu_init_scope(self, arg):
        '''
         xpsMtuInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMtuInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsMtuInit
    #/********************************************************************************/
    def do_mtu_init(self, arg):
        '''
         xpsMtuInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMtuInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMtuDeInitScope
    #/********************************************************************************/
    def do_mtu_de_init_scope(self, arg):
        '''
         xpsMtuDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMtuDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsMtuDeInit
    #/********************************************************************************/
    def do_mtu_de_init(self, arg):
        '''
         xpsMtuDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMtuDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuAddDevice
    #/********************************************************************************/
    def do_mtu_add_device(self, arg):
        '''
         xpsMtuAddDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMtuAddDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuRemoveDevice
    #/********************************************************************************/
    def do_mtu_remove_device(self, arg):
        '''
         xpsMtuRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMtuRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuSetInterfaceMtuSize
    #/********************************************************************************/
    def do_mtu_set_interface_mtu_size(self, arg):
        '''
         xpsMtuSetInterfaceMtuSize: Enter [ devId,intfId,mtuSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,mtuSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, mtuSize=%d' % (args[0],args[1],args[2]))
            ret = xpsMtuSetInterfaceMtuSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuUpdateInterfaceMtuSize
    #/********************************************************************************/
    def do_mtu_update_interface_mtu_size(self, arg):
        '''
         xpsMtuUpdateInterfaceMtuSize: Enter [ devId,intfId,mtuSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,mtuSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, mtuSize=%d' % (args[0],args[1],args[2]))
            ret = xpsMtuUpdateInterfaceMtuSize(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuGetInterfaceMtuSize
    #/********************************************************************************/
    def do_mtu_get_interface_mtu_size(self, arg):
        '''
         xpsMtuGetInterfaceMtuSize: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mtuSize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsMtuGetInterfaceMtuSize(args[0],args[1],mtuSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuSize = %d' % (uint32_tp_value(mtuSize_Ptr_2)))
                pass
            delete_uint32_tp(mtuSize_Ptr_2)
    #/********************************************************************************/
    # command for xpsMtuGetMtuProfile
    #/********************************************************************************/
    def do_mtu_get_mtu_profile(self, arg):
        '''
         xpsMtuGetMtuProfile: Enter [ devId,profileIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mtuSize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileIdx=%d' % (args[0],args[1]))
            ret = xpsMtuGetMtuProfile(args[0],args[1],mtuSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuSize = %d' % (uint32_tp_value(mtuSize_Ptr_2)))
                pass
            delete_uint32_tp(mtuSize_Ptr_2)
    #/********************************************************************************/
    # command for xpsMtuGetIpMtuProfile
    #/********************************************************************************/
    def do_mtu_get_ip_mtu_profile(self, arg):
        '''
         xpsMtuGetIpMtuProfile: Enter [ devId,profileIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,profileIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mtuSize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, profileIdx=%d' % (args[0],args[1]))
            ret = xpsMtuGetIpMtuProfile(args[0],args[1],mtuSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuSize = %d' % (uint32_tp_value(mtuSize_Ptr_2)))
                pass
            delete_uint32_tp(mtuSize_Ptr_2)
    #/********************************************************************************/
    # command for xpsMtuDisplayNhTable 
    #/********************************************************************************/
    def do_mtu_display_nh_table(self, arg):
        '''
         xpsMtuDisplayNhTable: Enter [ devId,egrPortNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,egrPortNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, profileIdx=%d' % (args[0],args[1]))
            ret = xpsMtuDisplayNhTable(args[0],args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuGetInterfaceMtuProfileIdx
    #/********************************************************************************/
    def do_mtu_get_interface_mtu_profile_idx(self, arg):
        '''
         xpsMtuGetInterfaceMtuProfileIdx: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mtuProfileIdx_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsMtuGetInterfaceMtuProfileIdx(args[0],args[1],mtuProfileIdx_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuProfileIdx = %d' % (uint32_tp_value(mtuProfileIdx_Ptr_2)))
                pass
            delete_uint32_tp(mtuProfileIdx_Ptr_2)
    #/********************************************************************************/
    # command for xpsMtuEnableMtuCheck
    #/********************************************************************************/
    def do_mtu_enable_mtu_check(self, arg):
        '''
         xpsMtuEnableMtuCheck: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMtuEnableMtuCheck(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMtuIsMtuCheckEnabled
    #/********************************************************************************/
    def do_mtu_is_mtu_check_enabled(self, arg):
        '''
         xpsMtuIsMtuCheckEnabled: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMtuIsMtuCheckEnabled(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_1)))
                pass
            delete_uint32_tp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMtuSetMtuErrorPktCmd
    #/********************************************************************************/
    def do_mtu_set_mtu_error_pkt_cmd(self, arg):
        '''
         xpsMtuSetMtuErrorPktCmd: Enter [ devId,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, pktCmd=%d' % (args[0],args[1]))
            ret = xpsMtuSetMtuErrorPktCmd(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
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
