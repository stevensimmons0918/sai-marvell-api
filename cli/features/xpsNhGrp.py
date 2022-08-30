#!/usr/bin/env python
#  xpsNhGrp.py
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
# The class object for xpsNhGrp operations
#/**********************************************************************************/

class xpsNhGrpObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsL3InitRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_init_route_next_hop_group_scope(self, arg):
        '''
         xpsL3InitRouteNextHopGroupScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3InitRouteNextHopGroupScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsL3InitRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_init_route_next_hop_group(self, arg):
        '''
         xpsL3InitRouteNextHopGroup: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3InitRouteNextHopGroup()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3DeInitRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_de_init_route_next_hop_group_scope(self, arg):
        '''
         xpsL3DeInitRouteNextHopGroupScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3DeInitRouteNextHopGroupScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsL3DeInitRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_de_init_route_next_hop_group(self, arg):
        '''
         xpsL3DeInitRouteNextHopGroup: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3DeInitRouteNextHopGroup()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3SetRouteNextHopGlobalMaxSizeScope
    #/********************************************************************************/
    def do_l3_set_route_next_hop_global_max_size_scope(self, arg):
        '''
         xpsL3SetRouteNextHopGlobalMaxSizeScope: Enter [ scopeId,size ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,size ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, size=%d' % (args[0],args[1]))
            ret = xpsL3SetRouteNextHopGlobalMaxSizeScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsL3SetRouteNextHopGlobalMaxSize
    #/********************************************************************************/
    def do_l3_set_route_next_hop_global_max_size(self, arg):
        '''
         xpsL3SetRouteNextHopGlobalMaxSize: Enter [ size ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ size ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, size=%d' % (args[0]))
            ret = xpsL3SetRouteNextHopGlobalMaxSize(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    #/********************************************************************************/
    # command for xpsL3CreateRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_create_route_next_hop_group_scope(self, arg):
        '''
         xpsL3CreateRouteNextHopGroupScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            pNhGrpId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3CreateRouteNextHopGroupScope(args[0],pNhGrpId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pNhGrpId = %d' % (uint32_tp_value(pNhGrpId_Ptr_1)))
                pass
            delete_uint32_tp(pNhGrpId_Ptr_1)
			
    # command for xpsL3CreateRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_create_route_next_hop_group(self, arg):
        '''
         xpsL3CreateRouteNextHopGroup: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            pNhGrpId_Ptr_0 = new_uint32_tp()
            #print('Input Arguments are' % ())
            ret = xpsL3CreateRouteNextHopGroup(pNhGrpId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pNhGrpId = %d' % (uint32_tp_value(pNhGrpId_Ptr_0)))
                pass
            delete_uint32_tp(pNhGrpId_Ptr_0)
    #/********************************************************************************/
    # command for xpsL3DestroyRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_destroy_route_next_hop_group(self, arg):
        '''
         xpsL3DestroyRouteNextHopGroup: Enter [ devId,nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhGrpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, nhGrpId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyRouteNextHopGroup(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddNextHopRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_add_next_hop_route_next_hop_group(self, arg):
        '''
         xpsL3AddNextHopRouteNextHopGroup: Enter [ devId,nhGrpId,nhId,weight ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhGrpId,nhId,weight ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, nhGrpId=%d, nhId=%d, weight=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3AddNextHopRouteNextHopGroup(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveNextHopRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_remove_next_hop_route_next_hop_group(self, arg):
        '''
         xpsL3RemoveNextHopRouteNextHopGroup: Enter [ devId,nhGrpId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhGrpId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, nhGrpId=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3RemoveNextHopRouteNextHopGroup(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
     #/********************************************************************************/
    # command for xpsL3GetCountRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_get_count_route_next_hop_group_scope(self, arg):
        '''
         xpsL3GetCountRouteNextHopGroupScope: Enter [ scopeId,nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhGrpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pCount_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, nhGrpId=%d' % (args[0],args[1]))
            ret = xpsL3GetCountRouteNextHopGroupScope(args[0],args[1],pCount_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pCount = %d' % (uint32_tp_value(pCount_Ptr_2)))
                pass
            delete_uint32_tp(pCount_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3GetNextNextHopRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_get_next_next_hop_route_next_hop_group_scope(self, arg):
        '''
         xpsL3GetNextNextHopRouteNextHopGroupScope: Enter [ scopeId,nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhGrpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pNhId_Ptr_2 = new_uint32_tp()
            pNextNhId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, nhGrpId=%d' % (args[0],args[1]))
            ret = xpsL3GetNextNextHopRouteNextHopGroupScope(args[0],args[1],pNhId_Ptr_2,pNextNhId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pNhId = %d' % (uint32_tp_value(pNhId_Ptr_2)))
                print('pNextNhId = %d' % (uint32_tp_value(pNextNhId_Ptr_3)))
                pass
            delete_uint32_tp(pNextNhId_Ptr_3)
            delete_uint32_tp(pNhId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3ExistsRouteNextHopGroupScope
    #/********************************************************************************/
    def do_l3_exists_route_next_hop_group_scope(self, arg):
        '''
         xpsL3ExistsRouteNextHopGroupScope: Enter [ scopeId,nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhGrpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, nhGrpId=%d' % (args[0],args[1]))
            ret = xpsL3ExistsRouteNextHopGroupScope(args[0],args[1])
            if ret == 0:
                print('NH group doesnt exist')
            else:
                print('NH group exists')
    #/********************************************************************************/
    # command for xpsL3GetCountRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_get_count_route_next_hop_group(self, arg):
        '''
         xpsL3GetCountRouteNextHopGroup: Enter [ nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhGrpId ]')
        else:
            args[0] = int(args[0])
            pCount_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, nhGrpId=%d' % (args[0]))
            ret = xpsL3GetCountRouteNextHopGroup(args[0],pCount_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pCount = %d' % (uint32_tp_value(pCount_Ptr_1)))
                pass
            delete_uint32_tp(pCount_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3GetNextNextHopRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_get_next_next_hop_route_next_hop_group(self, arg):
        '''
         xpsL3GetNextNextHopRouteNextHopGroup: Enter [ nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhGrpId ]')
        else:
            args[0] = int(args[0])
            pNhId_Ptr_1 = new_uint32_tp()
            pNextNhId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, nhGrpId=%d' % (args[0]))
            ret = xpsL3GetNextNextHopRouteNextHopGroup(args[0],pNhId_Ptr_1,pNextNhId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pNhId = %d' % (uint32_tp_value(pNhId_Ptr_1)))
                print('pNextNhId = %d' % (uint32_tp_value(pNextNhId_Ptr_2)))
                pass
            delete_uint32_tp(pNextNhId_Ptr_2)
            delete_uint32_tp(pNhId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3SetRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_set_route_next_hop_group(self, arg):
        '''
         xpsL3SetRouteNextHopGroup: Enter [ devId,nhId,pktCmd,serviceInstId,propTTL,nextHop ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhId,pktCmd,serviceInstId,propTTL,nextHop ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3NextHopEntry_t_Ptr = new_xpsL3NextHopEntry_tp()
            xpsL3NextHopEntry_t_Ptr.pktCmd = eval(args[2])
            xpsL3NextHopEntry_t_Ptr.serviceInstId = int(args[3])
            xpsL3NextHopEntry_t_Ptr.propTTL = int(args[4])
            xpsL3NextHopEntry_t_Ptr.nextHop = int(args[5])
            #print('Input Arguments are, devId=%d, nhId=%d, pktCmd=%d, serviceInstId=%d, propTTL=%d, nextHop=%d' % (args[0],args[1],xpsL3NextHopEntry_t_Ptr.pktCmd,xpsL3NextHopEntry_t_Ptr.serviceInstId,xpsL3NextHopEntry_t_Ptr.propTTL,xpsL3NextHopEntry_t_Ptr.nextHop))
            ret = xpsL3SetRouteNextHopGroup(args[0],args[1],xpsL3NextHopEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsL3NextHopEntry_t_Ptr.pktCmd))
                print('serviceInstId = %d' % (xpsL3NextHopEntry_t_Ptr.serviceInstId))
                print('propTTL = %d' % (xpsL3NextHopEntry_t_Ptr.propTTL))
                print('nextHop = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop))
                pass
            delete_xpsL3NextHopEntry_tp(xpsL3NextHopEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3ExistsRouteNextHopGroup
    #/********************************************************************************/
    def do_l3_exists_route_next_hop_group(self, arg):
        '''
         xpsL3ExistsRouteNextHopGroup: Enter [ nhGrpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhGrpId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, nhGrpId=%d' % (args[0]))
            ret = xpsL3ExistsRouteNextHopGroup(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetRouteNextHopNextHopGroup
    #/********************************************************************************/
    def do_l3_set_route_next_hop_next_hop_group(self, arg):
        '''
         xpsL3SetRouteNextHopNextHopGroup: Enter [ devId,nhId,pktCmd,serviceInstId,propTTL,nextHop ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhId,pktCmd,serviceInstId,propTTL,nextHop ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3NextHopEntry_t_Ptr = new_xpsL3NextHopEntry_tp()
            xpsL3NextHopEntry_t_Ptr.pktCmd = eval(args[2])
            xpsL3NextHopEntry_t_Ptr.serviceInstId = int(args[3])
            xpsL3NextHopEntry_t_Ptr.propTTL = int(args[4])
            xpsL3NextHopEntry_t_Ptr.nextHop = int(args[5])
            #print('Input Arguments are, devId=%d, nhId=%d, pktCmd=%d, serviceInstId=%d, propTTL=%d, nextHop=%d' % (args[0],args[1],xpsL3NextHopEntry_t_Ptr.pktCmd,xpsL3NextHopEntry_t_Ptr.serviceInstId,xpsL3NextHopEntry_t_Ptr.propTTL,xpsL3NextHopEntry_t_Ptr.nextHop))
            ret = xpsL3SetRouteNextHopNextHopGroup(args[0],args[1],xpsL3NextHopEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsL3NextHopEntry_t_Ptr.pktCmd))
                print('serviceInstId = %d' % (xpsL3NextHopEntry_t_Ptr.serviceInstId))
                print('propTTL = %d' % (xpsL3NextHopEntry_t_Ptr.propTTL))
                print('nextHop = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop))
                pass
            delete_xpsL3NextHopEntry_tp(xpsL3NextHopEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3DestroyRouteNextHopNextHopGroup
    #/********************************************************************************/
    def do_l3_destroy_route_next_hop_next_hop_group(self, arg):
        '''
         xpsL3DestroyRouteNextHopNextHopGroup: Enter [ devId,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, nhEcmpSize=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3DestroyRouteNextHopNextHopGroup(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3GetMaxEcmpSize
    #/********************************************************************************/
    def do_l3_get_max_ecmp_size(self, arg):
        '''
         xpsL3GetMaxEcmpSize: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            pCount_Ptr_2 = new_uint32_tp()
            ret = xpsL3GetMaxEcmpSize(args[0],pCount_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pCount = %d' % (uint32_tp_value(pCount_Ptr_2)))
                pass
            delete_uint32_tp(pCount_Ptr_2)

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

