#!/usr/bin/env python
#  xpsTunnel.py
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
# The class object for xpsTunnel operations
#/**********************************************************************************/

class xpsTunnelObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    def do_tunnel_init_scope(self, arg):
        '''
         xpsTunnelInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsTunnelInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsTunnelDeInitScope
    #/********************************************************************************/
    def do_tunnel_de_init_scope(self, arg):
        '''
         xpsTunnelDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsTunnelDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    # command for xpsTunnelInit
    #/********************************************************************************/
    def do_tunnel_init(self, arg):
        '''
         xpsTunnelInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsTunnelInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsTunnelDeInit
    #/********************************************************************************/
    def do_tunnel_de_init(self, arg):
        '''
         xpsTunnelDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsTunnelDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelSetTnlIdRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_set_tnl_id_rehash_level(self, arg):
        '''
         xpsIpTunnelSetTnlIdRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsIpTunnelSetTnlIdRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGetTnlIdRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_get_tnl_id_rehash_level(self, arg):
        '''
         xpsIpTunnelGetTnlIdRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIpTunnelGetTnlIdRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsIpTunnelSetLocalTnlRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_set_local_tnl_rehash_level(self, arg):
        '''
         xpsIpTunnelSetLocalTnlRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsIpTunnelSetLocalTnlRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGetLocalTnlRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_get_local_tnl_rehash_level(self, arg):
        '''
         xpsIpTunnelGetLocalTnlRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIpTunnelGetLocalTnlRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsIpTunnelSetTnlTerminationRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_set_tnl_termination_rehash_level(self, arg):
        '''
         xpsIpTunnelSetTnlTerminationRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsIpTunnelSetTnlTerminationRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGetTnlTerminationRehashLevel
    #/********************************************************************************/
    def do_ip_tunnel_get_tnl_termination_rehash_level(self, arg):
        '''
         xpsIpTunnelGetTnlTerminationRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIpTunnelGetTnlTerminationRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsMplsSetTnlTerminationRehashLevel
    #/********************************************************************************/
    def do_mpls_set_tnl_termination_rehash_level(self, arg):
        '''
         xpsMplsSetTnlTerminationRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMplsSetTnlTerminationRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsGetTnlTerminationRehashLevel
    #/********************************************************************************/
    def do_mpls_get_tnl_termination_rehash_level(self, arg):
        '''
         xpsMplsGetTnlTerminationRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMplsGetTnlTerminationRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsTunnelAddDevice
    #/********************************************************************************/
    def do_tunnel_add_device(self, arg):
        '''
         xpsTunnelAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsTunnelAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsTunnelRemoveDevice
    #/********************************************************************************/
    def do_tunnel_remove_device(self, arg):
        '''
         xpsTunnelRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTunnelRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsTunnelEnableTermination
    #/********************************************************************************/
    def do_tunnel_enable_termination(self, arg):
        '''
         xpsTunnelEnableTermination: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTunnelEnableTermination(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsTunnelDisableTermination
    #/********************************************************************************/
    def do_tunnel_disable_termination(self, arg):
        '''
         xpsTunnelDisableTermination: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTunnelDisableTermination(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsRegisterRemoteVtepLearnHandler
    #/********************************************************************************/
    def do_register_remote_vtep_learn_handler(self, arg):
        '''
         xpsRegisterRemoteVtepLearnHandler: Enter [ func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ func ]')
        else:
            args[0] = int(args[0])
            user_data_Ptr_1 = new_voidp()
            #print('Input Arguments are, func=%d' % (args[0]))
            ret = xpsRegisterRemoteVtepLearnHandler(args[0],user_data_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('user_data = %d' % (voidp_value(user_data_Ptr_1)))
                pass
            delete_voidp(user_data_Ptr_1)

    #/********************************************************************************/
    # command for xpsUnregisterRemoteVtepLearnHandler
    #/********************************************************************************/
    def do_unregister_remote_vtep_learn_handler(self, arg):
        '''
         xpsUnregisterRemoteVtepLearnHandler: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsUnregisterRemoteVtepLearnHandler()
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

