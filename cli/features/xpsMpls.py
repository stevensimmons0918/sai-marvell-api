#!/usr/bin/env python
#  xpsMpls.py
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
# The class object for xpsMpls operations
#/**********************************************************************************/

class xpsMplsObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'


    #/********************************************************************************/
    # command for xpsMplsInitScope
    #/********************************************************************************/
    def do_mpls_init_scope(self, arg):
        '''
         xpsMplsInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMplsInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsMplsInit
    #/********************************************************************************/
    def do_mpls_init(self, arg):
        '''
         xpsMplsInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMplsInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsMplsDeInitScope
    #/********************************************************************************/
    def do_mpls_de_init_scope(self, arg):
        '''
         xpsMplsDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMplsDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsMplsDeInit
    #/********************************************************************************/
    def do_mpls_de_init(self, arg):
        '''
         xpsMplsDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMplsDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsAddDevice
    #/********************************************************************************/
    def do_mpls_add_device(self, arg):
        '''
         xpsMplsAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsMplsAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsRemoveDevice
    #/********************************************************************************/
    def do_mpls_remove_device(self, arg):
        '''
         xpsMplsRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMplsRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_mpls_create_tunnel_interface_scope(self, arg):
        '''
         xpsMplsCreateTunnelInterfaceScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            mplsTnlId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMplsCreateTunnelInterfaceScope(args[0],mplsTnlId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('mplsTnlId = %d' % (xpsInterfaceId_tp_value(mplsTnlId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(mplsTnlId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(mplsTnlId_Ptr_1)
			
    #/********************************************************************************/
    # command for xpsMplsCreateTunnelInterface
    #/********************************************************************************/
    def do_mpls_create_tunnel_interface(self, arg):
        '''
         xpsMplsCreateTunnelInterface: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            mplsTnlId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsMplsCreateTunnelInterface(mplsTnlId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('mplsTnlId = %d' % (xpsInterfaceId_tp_value(mplsTnlId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(mplsTnlId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(mplsTnlId_Ptr_0)

    #/********************************************************************************/
    # command for xpsMplsAddTunnelOriginationEntry
    #/********************************************************************************/
    def do_mpls_add_tunnel_origination_entry(self, arg):
        '''
        xpsMplsAddTunnelOriginationEntry: Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel,mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel,mplsTnlId ]')
        else:
            args[0] = int(args[0])
            xpsMplsTunnelKey_t_Ptr = new_xpsMplsTunnelKey_tp()
            xpsMplsTunnelKey_t_Ptr.isP2MP = int(args[1])
            xpsMplsTunnelKey_t_Ptr.numOfLables = int(args[2])
            xpsMplsTunnelKey_t_Ptr.firstLabel = int(args[3])
            xpsMplsTunnelKey_t_Ptr.secondLabel = int(args[4])
            args[5] = int(args[5])

            #print('Input Arguments are, devId=%d, isP2MP=%d, numOfLables=%d, firstLabel=%d, secondLabel=%d, mplsTnlId=%d' % (args[0],xpsMplsTunnelKey_t_Ptr.isP2MP,xpsMplsTunnelKey_t_Ptr.numOfLables,xpsMplsTunnelKey_t_Ptr.firstLabel,xpsMplsTunnelKey_t_Ptr.secondLabel,args[5]))
            ret = xpsMplsAddTunnelOriginationEntry(args[0],xpsMplsTunnelKey_t_Ptr,args[5])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isP2MP = %d' % (xpsMplsTunnelKey_t_Ptr.isP2MP))
                print('numOfLables = %d' % (xpsMplsTunnelKey_t_Ptr.numOfLables))
                print('firstLabel = %d' % (xpsMplsTunnelKey_t_Ptr.firstLabel))
                print('secondLabel = %d' % (xpsMplsTunnelKey_t_Ptr.secondLabel))
                pass
            delete_xpsMplsTunnelKey_tp(xpsMplsTunnelKey_t_Ptr)


    #/********************************************************************************/
    # command for xpsMplsAddTunnelTerminationEntry
    #/********************************************************************************/
    def do_mpls_add_tunnel_termination_entry(self, arg):
        '''
        xpsMplsAddTunnelTerminationEntry: Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]')
        else:
            args[0] = int(args[0])
            xpsMplsTunnelKey_t_Ptr = new_xpsMplsTunnelKey_tp()
            xpsMplsTunnelKey_t_Ptr.isP2MP = int(args[1])
            xpsMplsTunnelKey_t_Ptr.numOfLables = int(args[2])
            xpsMplsTunnelKey_t_Ptr.firstLabel = int(args[3])
            xpsMplsTunnelKey_t_Ptr.secondLabel = int(args[4])

            #print('Input Arguments are, devId=%d, isP2MP=%d, numOfLables=%d, firstLabel=%d, secondLabel=%d' % (args[0],xpsMplsTunnelKey_t_Ptr.isP2MP,xpsMplsTunnelKey_t_Ptr.numOfLables,xpsMplsTunnelKey_t_Ptr.firstLabel,xpsMp>
            ret = xpsMplsAddTunnelTerminationEntry(args[0],xpsMplsTunnelKey_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isP2MP = %d' % (xpsMplsTunnelKey_t_Ptr.isP2MP))
                print('numOfLables = %d' % (xpsMplsTunnelKey_t_Ptr.numOfLables))
                print('firstLabel = %d' % (xpsMplsTunnelKey_t_Ptr.firstLabel))
                print('secondLabel = %d' % (xpsMplsTunnelKey_t_Ptr.secondLabel))
                pass
            delete_xpsMplsTunnelKey_tp(xpsMplsTunnelKey_t_Ptr)

    #/********************************************************************************/
    # command for xpsMplsRemoveTunnelOriginationEntry
    #/********************************************************************************/
    def do_mpls_remove_tunnel_origination_entry(self, arg):
        '''
        xpsMplsRemoveTunnelOriginationEntry: Enter [ devId,mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsTnlId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mplsTnlId=%d' % (args[0],args[1]))
            ret = xpsMplsRemoveTunnelOriginationEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsRemoveTunnelTerminationEntry
    #/********************************************************************************/
    def do_mpls_remove_tunnel_termination_entry(self, arg):
        '''
        xpsMplsRemoveTunnelTerminationEntry: Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]')
        else:
            args[0] = int(args[0])
            xpsMplsTunnelKey_t_Ptr = new_xpsMplsTunnelKey_tp()
            xpsMplsTunnelKey_t_Ptr.isP2MP = int(args[1])
            xpsMplsTunnelKey_t_Ptr.numOfLables = int(args[2])
            xpsMplsTunnelKey_t_Ptr.firstLabel = int(args[3])
            xpsMplsTunnelKey_t_Ptr.secondLabel = int(args[4])

            #print('Input Arguments are, devId=%d, isP2MP=%d, numOfLables=%d, firstLabel=%d, secondLabel=%d' % (args[0],xpsMplsTunnelKey_t_Ptr.isP2MP,xpsMplsTunnelKey_t_Ptr.numOfLables,xpsMplsTunnelKey_t_Ptr.firstLabel,xpsMp>
            ret = xpsMplsRemoveTunnelTerminationEntry(args[0],xpsMplsTunnelKey_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isP2MP = %d' % (xpsMplsTunnelKey_t_Ptr.isP2MP))
                print('numOfLables = %d' % (xpsMplsTunnelKey_t_Ptr.numOfLables))
                print('firstLabel = %d' % (xpsMplsTunnelKey_t_Ptr.firstLabel))
                print('secondLabel = %d' % (xpsMplsTunnelKey_t_Ptr.secondLabel))
                pass
            delete_xpsMplsTunnelKey_tp(xpsMplsTunnelKey_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_mpls_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsMplsDestroyTunnelInterfaceScope: Enter [ scopeId,mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,mplsTnlId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, mplsTnlId=%d' % (args[0],args[1]))
            ret = xpsMplsDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsDestroyTunnelInterface
    #/********************************************************************************/
    def do_mpls_destroy_tunnel_interface(self, arg):
        '''
         xpsMplsDestroyTunnelInterface: Enter [ mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ mplsTnlId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, mplsTnlId=%d' % (args[0]))
            ret = xpsMplsDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsSetLabelEntryRehashLevel
    #/********************************************************************************/
    def do_mpls_set_label_entry_rehash_level(self, arg):
        '''
         xpsMplsSetLabelEntryRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMplsSetLabelEntryRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsGetLabelEntryRehashLevel
    #/********************************************************************************/
    def do_mpls_get_label_entry_rehash_level(self, arg):
        '''
         xpsMplsGetLabelEntryRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMplsGetLabelEntryRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMplsGetTunnelConfig
    #/********************************************************************************/
    def do_mpls_get_tunnel_config(self, arg):
        '''
        xpsMplsGetTunnelConfig: Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel ]')
        else:
            args[0] = int(args[0])
            xpsMplsTunnelKey_t_Ptr = new_xpsMplsTunnelKey_tp()
            xpsMplsTunnelKey_t_Ptr.isP2MP = int(args[1])
            xpsMplsTunnelKey_t_Ptr.numOfLables = int(args[2])
            xpsMplsTunnelKey_t_Ptr.firstLabel = int(args[3])
            xpsMplsTunnelKey_t_Ptr.secondLabel = int(args[4])
            tunnelParams_Ptr_5 = new_xpsMplsTunnelParams_tp()

            #print('Input Arguments are, devId=%d, isP2MP=%d, numOfLables=%d, firstLabel=%d, secondLabel=%d' % (args[0],xpsMplsTunnelKey_t_Ptr.isP2MP,xpsMplsTunnelKey_t_Ptr.numOfLables,xpsMplsTunnelKey_t_Ptr.firstLabel,xpsMplsMplsTunnelKey_t_Ptr.secondLabel))
            ret = xpsMplsGetTunnelConfig(args[0],xpsMplsTunnelKey_t_Ptr,tunnelParams_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('p2pLabelTnl.propTTL = %d' % (tunnelParams_Ptr_5.p2pLabelTnl.propTTL))
                print('p2pLabelTnl.countMode = %d' % (tunnelParams_Ptr_5.p2pLabelTnl.countMode))
                print('p2pLabelTnl.cntId = %d' % (tunnelParams_Ptr_5.p2pLabelTnl.cntId))
                print('p2mpLabelTnl.propTTL = %d' % (tunnelParams_Ptr_5.p2mpLabelTnl.propTTL))
                print('p2mpLabelTnl.countMode = %d' % (tunnelParams_Ptr_5.p2mpLabelTnl.countMode))
                print('p2mpLabelTnl.cntId = %d' % (tunnelParams_Ptr_5.p2mpLabelTnl.cntId))
                print('p2mpLabelTnl.isBudNode = %d' % (tunnelParams_Ptr_5.p2mpLabelTnl.isBudNode))
                pass
            delete_xpsMplsTunnelParams_tp(tunnelParams_Ptr_5)
            delete_xpsMplsTunnelKey_tp(xpsMplsTunnelKey_t_Ptr)

    #/********************************************************************************/
    # command for xpsMplsSetTunnelConfig
    #/********************************************************************************/
    def do_mpls_set_tunnel_config(self, arg):
        '''
        xpsMplsSetTunnelConfig: Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel,p2pLabelTnl.propTTL,countMode,cntId,p2mpLabelTnl.propTTL,countMode,cntId,isBudNode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 12
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isP2MP,numOfLables,firstLabel,secondLabel,p2pLabelTnl.propTTL,countMode,cntId,p2mpLabelTnl.propTTL,countMode,cntId,isBudNode ]')
        else:
            args[0] = int(args[0])
            xpsMplsTunnelKey_t_Ptr = new_xpsMplsTunnelKey_tp()
            xpsMplsTunnelKey_t_Ptr.isP2MP = int(args[1])
            xpsMplsTunnelKey_t_Ptr.numOfLables = int(args[2])
            xpsMplsTunnelKey_t_Ptr.firstLabel = int(args[3])
            xpsMplsTunnelKey_t_Ptr.secondLabel = int(args[4])
            tunnelParams_Ptr_2 = new_xpsMplsTunnelParams_tp()
            tunnelParams_Ptr_2.p2pLabelTnl.propTTL = int(args[5]);
            tunnelParams_Ptr_2.p2pLabelTnl.countMode = int(args[6]);
            tunnelParams_Ptr_2.p2pLabelTnl.cntId = int(args[7]);
            tunnelParams_Ptr_2.p2mpLabelTnl.propTTL = int(args[8]);
            tunnelParams_Ptr_2.p2mpLabelTnl.countMode = int(args[9]);
            tunnelParams_Ptr_2.p2mpLabelTnl.cntId = int(args[10]);
            tunnelParams_Ptr_2.p2mpLabelTnl.isBudNode = int(args[11]);

            #print('Input Arguments are, devId=%d, isP2MP=%d, numOfLables=%d, firstLabel=%d, secondLabel=%d p2pLabelTnl.propTTL=%d, countMode=%d, cntId=%d, p2mpLabelTnl.propTTL=%d, countMode=%d , cntId=%d, isBudNode=%d ' % (args[0],xpsMplsTunnelKey_t_Ptr.isP2MP,xpsMplsTunnelKey_t_Ptr.numOfLables,xpsMplsTunnelKey_t_Ptr.firstLabel,xpsMplsTunnelKey_t_Ptr.secondLabel, tunnelParams_Ptr_2.p2pLabelTnl.propTTL,tunnelParams_Ptr_2.p2pLabelTnl.countMode,tunnelParams_Ptr_2.p2pLabelTnl.cntId,tunnelParams_Ptr_2.p2mpLabelTnl.propTTL,tunnelParams_Ptr_2.p2mpLabelTnl.countMode,tunnelParams_Ptr_2.p2mpLabelTnl.cntId,tunnelParams_Ptr_2.p2mpLabelTnl.isBudNode))
            ret = xpsMplsSetTunnelConfig(args[0],xpsMplsTunnelKey_t_Ptr,tunnelParams_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsMplsTunnelParams_tp(tunnelParams_Ptr_2)
            delete_xpsMplsTunnelKey_tp(xpsMplsTunnelKey_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsSetTunnelNextHopData
    #/********************************************************************************/
    def do_mpls_set_tunnel_next_hop_data(self, arg):
        '''
         xpsMplsSetTunnelNextHopData: Enter [ devId,mplsTnlId,nextHopId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsTnlId,nextHopId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, mplsTnlId=%d, nextHopId=%d' % (args[0],args[1],args[2]))
            ret = xpsMplsSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_mpls_update_tunnel_next_hop_data(self, arg):
        '''
         xpsMplsUpdateTunnelNextHopData: Enter [ devId,mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsTnlId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mplsTnlId=%d' % (args[0],args[1]))
            ret = xpsMplsUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsSetVpnConfig
    #/********************************************************************************/
    def do_mpls_set_vpn_config(self, arg):
        '''
         xpsMplsSetVpnConfig: Enter [ devId,vpnLabel,flags,pktCmd,countMode,cntId,paclId,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,flags,pktCmd,countMode,cntId,paclId,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMplsVpnParams_t_Ptr = new_xpsMplsVpnParams_tp()
            xpsMplsVpnParams_t_Ptr.flags = int(args[2])
            xpsMplsVpnParams_t_Ptr.pktCmd = eval(args[3])
            xpsMplsVpnParams_t_Ptr.countMode = int(args[4])
            xpsMplsVpnParams_t_Ptr.cntId = int(args[5])
            xpsMplsVpnParams_t_Ptr.paclId = int(args[6])
            xpsMplsVpnParams_t_Ptr.raclId = int(args[7])
            
            #print('Input Arguments are, devId=%d, vpnLabel=%d, flags=%d, pktCmd=%d, countMode=%d, cntId=%d, paclId=%d, raclId=%d' % (args[0],args[1],xpsMplsVpnParams_t_Ptr.flags,xpsMplsVpnParams_t_Ptr.pktCmd,xpsMplsVpnParams_t_Ptr.countMode,xpsMplsVpnParams_t_Ptr.cntId,xpsMplsVpnParams_t_Ptr.paclId,xpsMplsVpnParams_t_Ptr.raclId))
            ret = xpsMplsSetVpnConfig(args[0],args[1],xpsMplsVpnParams_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('flags = %d' % (xpsMplsVpnParams_t_Ptr.flags))
                print('pktCmd = %d' % (xpsMplsVpnParams_t_Ptr.pktCmd))
                print('countMode = %d' % (xpsMplsVpnParams_t_Ptr.countMode))
                print('cntId = %d' % (xpsMplsVpnParams_t_Ptr.cntId))
                print('paclId = %d' % (xpsMplsVpnParams_t_Ptr.paclId))
                print('raclId = %d' % (xpsMplsVpnParams_t_Ptr.raclId))
                pass
            delete_xpsMplsVpnParams_tp(xpsMplsVpnParams_t_Ptr)

    #/********************************************************************************/
    # command for xpsMplsGetVpnConfig
    #/********************************************************************************/
    def do_mpls_get_vpn_config(self, arg):
        '''
         xpsMplsGetVpnConfig: Enter [ devId,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMplsVpnParams_t_Ptr = new_xpsMplsVpnParams_tp()
            #print('Input Arguments are, devId=%d, vpnLabel=%d ' % (args[0],args[1]))
            ret = xpsMplsGetVpnConfig(args[0],args[1],xpsMplsVpnParams_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('flags = %d' % (xpsMplsVpnParams_t_Ptr.flags))
                print('pktCmd = %d' % (xpsMplsVpnParams_t_Ptr.pktCmd))
                print('countMode = %d' % (xpsMplsVpnParams_t_Ptr.countMode))
                print('cntId = %d' % (xpsMplsVpnParams_t_Ptr.cntId))
                print('paclId = %d' % (xpsMplsVpnParams_t_Ptr.paclId))
                print('raclId = %d' % (xpsMplsVpnParams_t_Ptr.raclId))
                pass
            delete_xpsMplsVpnParams_tp(xpsMplsVpnParams_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsAddVpnGreLooseModeEntry
    #/********************************************************************************/
    def do_mpls_add_vpn_gre_loose_mode_entry(self, arg):
        '''
         xpsMplsAddVpnGreLooseModeEntry: Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMplsVpnGreLooseModeParams_t_Ptr = new_xpsMplsVpnGreLooseModeParams_tp()
            xpsMplsVpnGreLooseModeParams_t_Ptr.countMode = eval(args[2])
            xpsMplsVpnGreLooseModeParams_t_Ptr.counterId = int(args[3])
            xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL = int(args[4])
            xpsMplsVpnGreLooseModeParams_t_Ptr.bdId = int(args[5])
            
            #print('Input Arguments are, devId=%d, vpnLabel=%d, countMode=%d, counterId=%d, propTTL=%d, bdId=%d' % (args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr.countMode,xpsMplsVpnGreLooseModeParams_t_Ptr.counterId,xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL,xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
            ret = xpsMplsAddVpnGreLooseModeEntry(args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('countMode = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.countMode))
                print('counterId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.counterId))
                print('propTTL = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL))
                print('bdId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
                pass
            delete_xpsMplsVpnGreLooseModeParams_tp(xpsMplsVpnGreLooseModeParams_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsRemoveVpnGreLooseModeEntry
    #/********************************************************************************/
    def do_mpls_remove_vpn_gre_loose_mode_entry(self, arg):
        '''
         xpsMplsRemoveVpnGreLooseModeEntry: Enter [ devId,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vpnLabel=%d' % (args[0],args[1]))
            ret = xpsMplsRemoveVpnGreLooseModeEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsSetVpnGreLooseModeConfig
    #/********************************************************************************/
    def do_mpls_set_vpn_gre_loose_mode_config(self, arg):
        '''
         xpsMplsSetVpnGreLooseModeConfig: Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMplsVpnGreLooseModeParams_t_Ptr = new_xpsMplsVpnGreLooseModeParams_tp()
            xpsMplsVpnGreLooseModeParams_t_Ptr.countMode = eval(args[2])
            xpsMplsVpnGreLooseModeParams_t_Ptr.counterId = int(args[3])
            xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL = int(args[4])
            xpsMplsVpnGreLooseModeParams_t_Ptr.bdId = int(args[5])
            
            #print('Input Arguments are, devId=%d, vpnLabel=%d, countMode=%d, counterId=%d, propTTL=%d, bdId=%d' % (args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr.countMode,xpsMplsVpnGreLooseModeParams_t_Ptr.counterId,xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL,xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
            ret = xpsMplsSetVpnGreLooseModeConfig(args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('countMode = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.countMode))
                print('counterId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.counterId))
                print('propTTL = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL))
                print('bdId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
                pass
            delete_xpsMplsVpnGreLooseModeParams_tp(xpsMplsVpnGreLooseModeParams_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsGetVpnGreLooseModeConfig
    #/********************************************************************************/
    def do_mpls_get_vpn_gre_loose_mode_config(self, arg):
        '''
         xpsMplsGetVpnGreLooseModeConfig: Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,countMode,counterId,propTTL,bdId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMplsVpnGreLooseModeParams_t_Ptr = new_xpsMplsVpnGreLooseModeParams_tp()
            xpsMplsVpnGreLooseModeParams_t_Ptr.countMode = eval(args[2])
            xpsMplsVpnGreLooseModeParams_t_Ptr.counterId = int(args[3])
            xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL = int(args[4])
            xpsMplsVpnGreLooseModeParams_t_Ptr.bdId = int(args[5])
            
            #print('Input Arguments are, devId=%d, vpnLabel=%d, countMode=%d, counterId=%d, propTTL=%d, bdId=%d' % (args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr.countMode,xpsMplsVpnGreLooseModeParams_t_Ptr.counterId,xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL,xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
            ret = xpsMplsGetVpnGreLooseModeConfig(args[0],args[1],xpsMplsVpnGreLooseModeParams_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('countMode = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.countMode))
                print('counterId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.counterId))
                print('propTTL = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.propTTL))
                print('bdId = %d' % (xpsMplsVpnGreLooseModeParams_t_Ptr.bdId))
                pass
            delete_xpsMplsVpnGreLooseModeParams_tp(xpsMplsVpnGreLooseModeParams_t_Ptr)
    #/********************************************************************************/
    # command for xpsMplsAddLabelEntry
    #/********************************************************************************/
    def do_mpls_add_label_entry(self, arg):
        '''
         xpsMplsAddLabelEntry: Enter [ devId,keyLabel,pktCmd,mirrorMask,countMode,counterId,propTTL,swapLabel,mplsOper,l3InterfaceId,macDa,egressIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 12
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyLabel,pktCmd,mirrorMask,countMode,counterId,propTTL,swapLabel,mplsOper,l3InterfaceId,macDa,egressIntfId ]')
        else:
            args[0] = int(args[0])
            xpsMplsLabelEntry_t_Ptr = new_xpsMplsLabelEntry_tp()
            xpsMplsLabelEntry_t_Ptr.keyLabel = int(args[1])
            xpsMplsLabelEntry_t_Ptr.pktCmd = eval(args[2])
            xpsMplsLabelEntry_t_Ptr.mirrorMask = int(args[3])
            xpsMplsLabelEntry_t_Ptr.countMode = eval(args[4])
            xpsMplsLabelEntry_t_Ptr.counterId = int(args[5])
            xpsMplsLabelEntry_t_Ptr.propTTL = int(args[6])
            xpsMplsLabelEntry_t_Ptr.swapLabel = int(args[7])
            xpsMplsLabelEntry_t_Ptr.mplsOper = eval(args[8])

            xpsMplsLabelEntry_t_Ptr.nextHopData.l3InterfaceId = int(args[9])
            macList = args[10].split(":")
            for i in range(5, -1, -1):
                xpsMplsLabelEntry_t_Ptr.nextHopData.macDa[5 - i] = int(macList[i], 16)
            xpsMplsLabelEntry_t_Ptr.nextHopData.egressIntfId = int(args[11])
            
            #print('Input Arguments are, devId=%d, keyLabel=%d, pktCmd=%d, mirrorMask=%d, countMode=%d, counterId=%d, propTTL=%d, swapLabel=%d, mplsOper=%d, l3InterfaceId=%d, macDa=%s, egressIntfId=%d' % (args[0],xpsMplsLabelEntry_t_Ptr.keyLabel,xpsMplsLabelEntry_t_Ptr.pktCmd,xpsMplsLabelEntry_t_Ptr.mirrorMask,xpsMplsLabelEntry_t_Ptr.countMode,xpsMplsLabelEntry_t_Ptr.counterId,xpsMplsLabelEntry_t_Ptr.propTTL,xpsMplsLabelEntry_t_Ptr.swapLabel,xpsMplsLabelEntry_t_Ptr.mplsOper,xpsMplsLabelEntry_t_Ptr.nextHopData.l3InterfaceId,args[10],xpsMplsLabelEntry_t_Ptr.nextHopData.egressIntfId))
            ret = xpsMplsAddLabelEntry(args[0],xpsMplsLabelEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('keyLabel = %d' % (xpsMplsLabelEntry_t_Ptr.keyLabel))
                print('pktCmd = %d' % (xpsMplsLabelEntry_t_Ptr.pktCmd))
                print('mirrorMask = %d' % (xpsMplsLabelEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMplsLabelEntry_t_Ptr.countMode))
                print('counterId = %d' % (xpsMplsLabelEntry_t_Ptr.counterId))
                print('propTTL = %d' % (xpsMplsLabelEntry_t_Ptr.propTTL))
                print('swapLabel = %d' % (xpsMplsLabelEntry_t_Ptr.swapLabel))
                print('mplsOper = %d' % (xpsMplsLabelEntry_t_Ptr.mplsOper))
                print('l3InterfaceId = %d' % (xpsMplsLabelEntry_t_Ptr.nextHopData.l3InterfaceId))
                print('macDa = %s' % (args[10]))
                print('egressIntfId = %d' % (xpsMplsLabelEntry_t_Ptr.nextHopData.egressIntfId))
                pass
            delete_xpsMplsLabelEntry_tp(xpsMplsLabelEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsMplsRemoveLabelEntry
    #/********************************************************************************/
    def do_mpls_remove_label_entry(self, arg):
        '''
         xpsMplsRemoveLabelEntry: Enter [ devId,mplsLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mplsLabel=%d' % (args[0],args[1]))
            ret = xpsMplsRemoveLabelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMplsGetLabelEntry
    #/********************************************************************************/
    def do_mpls_get_label_entry(self, arg):
        '''
         xpsMplsGetLabelEntry: Enter [ devId,mplsLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1]) 
            xpsMplsLabelEntry_t_Ptr = new_xpsMplsLabelEntry_tp()

            #print('Input Arguments are, devId=%d, mplsLabel=%d' % (args[0],args[1]))
            ret = xpsMplsGetLabelEntry(args[0],args[1],xpsMplsLabelEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('keyLabel = %d' % (xpsMplsLabelEntry_t_Ptr.keyLabel))
                print('pktCmd = %d' % (xpsMplsLabelEntry_t_Ptr.pktCmd))
                print('mirrorMask = %d' % (xpsMplsLabelEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMplsLabelEntry_t_Ptr.countMode))
                print('counterId = %d' % (xpsMplsLabelEntry_t_Ptr.counterId))
                print('propTTL = %d' % (xpsMplsLabelEntry_t_Ptr.propTTL))
                print('swapLabel = %d' % (xpsMplsLabelEntry_t_Ptr.swapLabel))
                print('mplsOper = %d' % (xpsMplsLabelEntry_t_Ptr.mplsOper))
                print('l3InterfaceId = %d' % (xpsMplsLabelEntry_t_Ptr.nextHopData.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsMplsLabelEntry_t_Ptr.nextHopData.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsMplsLabelEntry_t_Ptr.nextHopData.egressIntfId))
                pass
            delete_xpsMplsLabelEntry_tp(xpsMplsLabelEntry_t_Ptr)

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

