#!/usr/bin/env python
#  displayTables.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

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
sys.path.append(dirname + "/pl")
from cmd2x import Cmd

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *

#/**********************************************************************************/
# The class object for xpsIacl operations
#/**********************************************************************************/
mapxpIaclV4KeyByteMask = {
    0: 0x1,  #XP_IACL_KEY_TYPE_V4
    1: 0x1,  #XP_IACL_ID
    2: 0x3f, #XP_IACL_MAC_DA
    3: 0x3f, #XP_IACL_MAC_SA
    4: 0x3,  #XP_IACL_V4_ETHER_TYPE
    5: 0x3,  #XP_IACL_CTAG_VID_DEI_PCP
    6: 0x3,  #XP_IACL_STAG_VID_DEI_PCP
    7: 0xf,  #XP_IACL_DIP_V4
    8: 0xf,  #XP_IACL_SIP_V4
    9: 0x3,  #XP_IACL_L4_DEST_PORT
    10: 0x3, #XP_IACL_L4_SRC_PORT
    11: 0x3, #XP_IACL_IVIF
    12: 0x1, #XP_IACL_ICMP_CODE,
    13: 0x1, #XP_IACL_ICMP_MSG_TYPE
    14: 0x1, #XP_IACL_PROTOCOL
    15: 0x1, #XP_IACL_DSCP_HAS_CTAG_STAG
    16: 0x3, #XP_IACL_BD
    17: 0x1, #XP_IACL_TTL
    18: 0x1, #XP_IACL_PKTLEN
    19: 0x1, #XP_IACL_TCP_FLAGS
    20: 0x3, #XP_IACL_VRF_ID
    21: 0x1, #XP_IACL_TAG_FRAGMENT_INFO
}
mapxpIaclV6KeyByteMask = {
    0: 0x1,   #XP_IACL_KEY_TYPE_V6
    1: 0x1,   #XP_IACL_V6_ID
    2: 0xffff,#XP_IACL_DIP_V6
    3: 0xffff,#XP_IACL_SIP_V6
    4: 0x1,   #XP_IACL_NXT_HDR
    5: 0x3,   #XP_IACL_L4_V6_DEST_PORT
    6: 0x3,   #XP_IACL_L4_V6_SRC_PORT
    7: 0x1,   #XP_IACL_ICMP_V6_MSG_TYPE
    8: 0x1,   #XP_IACL_HOP_LIMIT
    9: 0x1,  #XP_IACL_ROUTER_MAC_ISTCP_ISUDP
    10: 0x1, #XP_IACL_V6_PROTOCOL,
    11: 0x1, #XP_IACL_V6_TCP_FLAGS,
    12: 0x1, #XP_IACL_V6_PKTLEN,
    13: 0x1, #XP_IACL_TC_ROUTERMAC,
};

#/*********************************************************************************************************/
# The class object for display table commands
#/*********************************************************************************************************/
class displayTableCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'
    #/*****************************************************************************************************/
    # Generic subRotine to perform displayTable checking with devId as static parameter
    #/*****************************************************************************************************/
    def display_table_with_devid_arg(self, args, mgrInstance):
        numOfValidEntries = new_uint32_tp()
        if (len(args) == 6):
            print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5]))
            eval( mgrInstance + '(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], args[5])')
        elif (len(args) == 5):
            print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
            eval( mgrInstance + '(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], 0)')
        elif (len(args) == 4):
            print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
            eval( mgrInstance + '(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0, 0)')
        elif (len(args) == 3):
            print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
            eval( mgrInstance + '(args[0], numOfValidEntries, args[1], args[2], None, 0, 0, 0)')
        elif (len(args) == 2):
            print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
            eval( mgrInstance + '(args[0], numOfValidEntries, args[1], 0, None, 0, 0, 0)')
        elif (len(args) == 1):
            print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0]))
            eval( mgrInstance + '(args[0], numOfValidEntries, 0, 0, None, 0, 0, 0)')

    #/*****************************************************************************************************/
    # Generic subRotine to perform displayTable checking with devId and type as static parameters
    #/*****************************************************************************************************/
    def display_table_with_devid_type_args(self, args, mgrInstance):
        if (xpDeviceMgr.instance().isDeviceValid(args[0])):
            numOfValidEntries = new_uint32_tp()
            if (len(args) == 7):
                print('Input Arguments are devId=%d, type=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                eval (mgrInstance + '(args[0], args[1], numOfValidEntries, args[2], args[3], None, args[4], args[5], args[6])')
            if (len(args) == 6):
                print('Input Arguments are devId=%d, type=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                eval(mgrInstance + '(args[0], args[1], numOfValidEntries, args[2], args[3], None, args[4], args[5], 0)')
            elif (len(args) == 5):
                print('Input Arguments are devId=%d, type=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                eval(mgrInstance + '(args[0], args[1], numOfValidEntries, args[2], args[3], None, args[4], 0, 0)')
            elif (len(args) == 4):
                print('Input Arguments are devId=%d, type=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                eval(mgrInstance + '(args[0], args[1], numOfValidEntries, args[2], args[3], None, 0, 0, 0)')
            elif (len(args) == 3):
                print('Input Arguments are devId=%d, type=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                eval(mgrInstance + '(args[0], args[1], numOfValidEntries, args[2], 0, None, 0, 0, 0)')
            elif (len(args) == 2):
                print('Input Arguments are devId=%d, type=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                eval(mgrInstance + '(args[0], args[1], numOfValidEntries, 0, 0, None, 0, 0, 0)')
            delete_uint32_tp(numOfValidEntries)
        else:
            print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display FDB table entries
    #/*****************************************************************************************************/
    def do_display_fdb(self, arg):
        'Display FDB entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            numOfValidEntries = new_uint32_tp()
            if (len(args) == 5):
                print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d' % (args[0], args[1], args[2], args[3], args[4]))
                eval( 'xpsFdbDisplayTable' + '(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4])')
            elif (len(args) == 4):
                print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=0' % (args[0], args[1], args[2], args[3]))
                eval( 'xpsFdbDisplayTable' + '(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0)')
            elif (len(args) == 3):
                print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0' % (args[0], args[1], args[2]))
                eval( 'xpsFdbDisplayTable' + '(args[0], numOfValidEntries, args[1], args[2], None, 0, 0)')
            elif (len(args) == 2):
                print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0' % (args[0], args[1]))
                eval( 'xpsFdbDisplayTable' + '(args[0], numOfValidEntries, args[1], 0, None, 0, 0)')
            elif (len(args) == 1):
                print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0' % (args[0]))
                eval( 'xpsFdbDisplayTable' + '(args[0], numOfValidEntries, 0, 0, None, 0, 0)')
            delete_uint32_tp(numOfValidEntries)

    #/*****************************************************************************************************/
    # command to display FDB table entries
    #/*****************************************************************************************************/
    def do_display_fdb_loop(self, arg):
        'Display FDB entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'

        for i in range(0, 10000000):
            self.do_display_fdb(arg)
            print('Iteration %d\n' % (i))
            time.sleep(1)

    #/*****************************************************************************************************/
    # command to display ipv4 route table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_route(self, arg):
        'Display ipv4 route entries, Enter Device-Id, [End-Index]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [ End-Index]'
        else:
            numOfValidEntries = new_uint32_tp()
            end_index = 0
            if (len(args) == 2):
                 print('Input Arguments are devId=%d, endIndex=%d' % (args[0], args[1]))
                 end_index = args[1]
            elif (len(args) == 1):
                 print('Input Arguments are devId=%d, endIndex=0' % (args[0]))
                 xpsDisplayRouteTable(args[0], 0, end_index)
                 delete_uint32_tp(numOfValidEntries)


    #/*****************************************************************************************************/
    # command to display ipv6 route table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_route(self, arg):
        'Display ipv6 route entries, Enter Device-Id, [End-Index]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [ End-Index]'
        else:
                numOfValidEntries = new_uint32_tp()
                end_index = 0
                if (len(args) == 2):
                    print('Input Arguments are devId=%d, endIndex=%d' % (args[0], args[1]))
                    end_index = args[1]
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, endIndex=0' % (args[0]))
                xpsDisplayRouteTable(args[0], 1, end_index)
                delete_uint32_tp(numOfValidEntries)

    #/*****************************************************************************************************/
    # command to display tunnel local vtep table entries
    #/*****************************************************************************************************/
    def do_display_xp70cust_tunnel_local_vtep(self, arg):
        'Display ipv6 route entries, Enter Device-Id, [End-Index]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [ End-Index]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                end_index = 0
                if (len(args) == 2):
                    print('Input Arguments are devId=%d, endIndex=%d' % (args[0], args[1]))
                    end_index = args[1]
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, endIndex=0' % (args[0]))
                xpsTunnelLocalVtepDisplayTable(args[0], numOfValidEntries)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display tunnel id table entries
    #/*****************************************************************************************************/
    def do_display_xp70cust_tunnel_id(self, arg):
        'Display ipv6 route entries, Enter Device-Id, [End-Index]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [ End-Index]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                end_index = 0
                if (len(args) == 2):
                    print('Input Arguments are devId=%d, endIndex=%d' % (args[0], args[1]))
                    end_index = args[1]
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, endIndex=0' % (args[0]))
                xpsTunnelTunnelIdDisplayTable(args[0], numOfValidEntries)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display Tunnel IVIF  table entries
    #/*****************************************************************************************************/
    def do_display_tunnel_ivif(self, arg):
        'Display Tunnel Ivif entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTunnelIvifDisplayTable')

    #/*****************************************************************************************************/
    # command to display Local VTEP Table  table entries
    #/*****************************************************************************************************/
    def do_display_tunnel_local_vtep(self, arg):
        'Display Local Tunnel VTEP entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                xpTunnelPl.instance().createMgr(xpTunnelIvifMgr_getId())
                tnlMgr = getTnlTablePtr(xpTunnelPl.instance().getMgr(xpTunnelIvifMgr_getId()))
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 6):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4]. args[5]))
                    tnlMgr.displayLocalVTEPTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], args[5])
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    tnlMgr.displayLocalVTEPTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0, 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                    tnlMgr.displayLocalVTEPTable(args[0], numOfValidEntries, args[1], args[2], None, 0, 0, 0)
                elif (len(args) == 2):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                    tnlMgr.displayLocalVTEPTable(args[0], numOfValidEntries, args[1], 0, None, 0, 0, 0)
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0]))
                    tnlMgr.displayLocalVTEPTable(args[0], numOfValidEntries, 0, 0, None, 0, 0, 0)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display local SID table entries
    #/*****************************************************************************************************/
    def do_display_local_sid(self, arg):
        'Display local Sid table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                xpTunnelPl.instance().createMgr(xpLocalSidMgr_getId())
                localSidMgr = getLocalSidTablePtr(xpTunnelPl.instance().getMgr(xpLocalSidMgr_getId()))
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 6):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4]. args[5]))
                    localSidMgr.displayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], args[5])
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    localSidMgr.displayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0, 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                    localSidMgr.displayTable(args[0], numOfValidEntries, args[1], args[2], None, 0, 0, 0)
                elif (len(args) == 2):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                    localSidMgr.displayTable(args[0], numOfValidEntries, args[1], 0, None, 0, 0, 0)
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0]))
                    localSidMgr.displayTable(args[0], numOfValidEntries, 0, 0, None, 0, 0, 0)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display Tunnel Id Table  table entries
    #/*****************************************************************************************************/
    def do_display_tunnel_id(self, arg):
        'Display Tunnel Id entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                xpTunnelPl.instance().createMgr(xpTunnelIvifMgr_getId())
                tnlMgr = getTnlTablePtr(xpTunnelPl.instance().getMgr(xpTunnelIvifMgr_getId()))
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 6):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4]. args[5]))
                    tnlMgr.displayTunnelIdTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], args[5])
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    tnlMgr.displayTunnelIdTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0, 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                    tnlMgr.displayTunnelIdTable(args[0], numOfValidEntries, args[1], args[2], None, 0, 0, 0)
                elif (len(args) == 2):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                    tnlMgr.displayTunnelIdTable(args[0], numOfValidEntries, args[1], 0, None, 0, 0, 0)
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0]))
                    tnlMgr.displayTunnelIdTable(args[0], numOfValidEntries, 0, 0, None, 0, 0, 0)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])


    #/*****************************************************************************************************/
    # command to display port Vlan table entries
    #/*****************************************************************************************************/
    def do_display_port_vlan(self, arg):
        'Display port Vlan entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpPortVlanIvifDisplayTable')

    #/*****************************************************************************************************/
    # command to display BD table entries
    #/*****************************************************************************************************/
    def do_display_bd(self, arg):
        'Display BD entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpBdDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 Host table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_host(self, arg):
        'Display Ipv4 Host entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4HostDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 Host table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_host(self, arg):
        'Display Ipv6 Host entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6HostDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 Route MC table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_route_mc(self, arg):
        'Display Ipv4 Route MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4RouteMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 Route MC S,G table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_route_sg_mc(self, arg):
        'Display Ipv4 Route SG MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4RouteSgMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 Route MC table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_route_mc(self, arg):
        'Display Ipv6 Route MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6RouteMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 Route MC S,G table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_route_sg_mc(self, arg):
        'Display Ipv6 Route SG MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6RouteSgMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Nh table entries
    #/*****************************************************************************************************/
    def do_display_nh(self, arg):
        'Display Nh entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpNhDisplayTable')

    #/****************************************************************************************************/
    # command to display MDT table entries
    #/*****************************************************************************************************/
    def do_display_mdt(self, arg):
        'Display MDT Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpMdtDisplayTable')

    #/*****************************************************************************************************/
    # command to display Port Config table entries
    #/*****************************************************************************************************/
    def do_display_port_config(self, arg):
        'Display Port Config Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpPortConfigDisplayTable')

    #/*****************************************************************************************************/
    # command to display Qos Map table entries
    #/*****************************************************************************************************/
    def do_display_qos_map(self, arg):
        'Display QOS Map Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpQosMapDisplayTable')

    #/*****************************************************************************************************/
    # command to display ICDT table entries
    #/*****************************************************************************************************/
    def do_display_iit(self, arg):
        'Display ICDT Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpHdrModificationDisplayIITTable')

    #/*****************************************************************************************************/
    # command to display egress filter table entries
    #/*****************************************************************************************************/
    def do_display_egress_filter(self, arg):
        'Display egress filter entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpEgressFilterDisplayTable')

    #/*****************************************************************************************************/
    # command to display egress bd table entries
    #/*****************************************************************************************************/
    def do_display_egress_bd(self, arg):
        'Display egress bd entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpEgressBdDisplayTable')

    #/*****************************************************************************************************/
    # command to display mpls label table entries
    #/*****************************************************************************************************/
    def do_display_mpls_label(self, arg):
        'Display MPLS Label entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpMplsLabelDisplayTable')

    #/*****************************************************************************************************/
    # command to display mpls tunnel table entries
    #/*****************************************************************************************************/
    def do_display_mpls_tunnel(self, arg):
        'Display MPLS tunnel entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpMplsTunnelDisplayTable')

    #/*****************************************************************************************************/
    # command to display trunk resolution table entries
    #/*****************************************************************************************************/
    def do_display_trunk_resolution(self, arg):
        'Display Trunk Resolution entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTrunkResolutionDisplayTable')

    #/*****************************************************************************************************/
    # command to display h1 counter table entries
    #/*****************************************************************************************************/
    def do_display_h1_counter(self, arg):
        'Display H1 Counter entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpH1CounterDisplayTable')

    #/*****************************************************************************************************/
    # command to display aqm pfl table entries
    #/*****************************************************************************************************/
    def do_display_aqm_pfl(self, arg):
        'Display Aqm Pfl entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpAqmPflDisplayTable')

    #/*****************************************************************************************************/
    # command to display aqm q pfl table entries
    #/*****************************************************************************************************/
    def do_display_aqm_q_pfl(self, arg):
        'Display Aqm Q Pfl entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpAqmQPflDisplayTable')

    #/*****************************************************************************************************/
    # command to display control mac table entries
    #/*****************************************************************************************************/
    def do_display_control_mac(self, arg):
        'Display Control MAC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpControlMacDisplayTable')

    #/*****************************************************************************************************/
    # command to display eq cfg table entries
    #/*****************************************************************************************************/
    def do_display_eq_cfg(self, arg):
        'Display EQ CFG entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpEqCfgDisplayTable')

    #/*****************************************************************************************************/
    # command to display reason code table entries
    #/*****************************************************************************************************/
    def do_display_reason_code(self, arg):
        'Display Reason Code entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpReasonCodeDisplayTable')

    #/*****************************************************************************************************/
    # command to display NAT IPV4 table entries
    #/*****************************************************************************************************/
    def do_display_nat_ipv4(self, arg):
        'Display NAT IPV4 entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpNATIpv4DisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 Bridge Mc table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_bridge_mc(self, arg):
        'Display IPV4 Bridge MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4BridgeMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 Bridge S,G Mc table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_bridge_sg_mc(self, arg):
        'Display IPV4 Bridge SG MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4BridgeSgMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 Bridge Mc table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_bridge_mc(self, arg):
        'Display IPV6 Bridge MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6BridgeMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 Bridge S,G Mc table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_bridge_sg_mc(self, arg):
        'Display IPV6 Bridge SG MC entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6BridgeSgMcDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv4 PIM BiDir RPF table entries
    #/*****************************************************************************************************/
    def do_display_ipv4_pim_bidir_rpf(self, arg):
        'Display Ipv4 PIM BiDir RPF entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv4PIMBiDirRPFDisplayTable')

    #/*****************************************************************************************************/
    # command to display Ipv6 PIM BiDir RPF table entries
    #/*****************************************************************************************************/
    def do_display_ipv6_pim_bidir_rpf(self, arg):
        'Display Ipv6 PIM BiDir RPF entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpIpv6PIMBiDirRPFDisplayTable')

    #/*****************************************************************************************************/
    # command to display Q Mapping table entries
    #/*****************************************************************************************************/
    def do_display_q_mapping(self, arg):
        'Display Q Mapping entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpQmappingDisplayTable')

    #/*****************************************************************************************************/
    # command to Q Counter table entries
    #/*****************************************************************************************************/
    def do_display_q_counter(self, arg):
        'Display Q Counter entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpQCounterDisplayTable')

    #/*****************************************************************************************************/
    # command to Port Mapping Cfg table entries
    #/*****************************************************************************************************/
    def do_display_port_mapping_cfg(self, arg):
        'Display Port Mapping Cfg entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpPfcMapCfgDisplayTable')

    #/*****************************************************************************************************/
    # command to Port Counter table entries
    #/*****************************************************************************************************/
    def do_display_port_counter(self, arg):
        'Display Port Counter entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpPortCounterDisplayTable')

    #/*****************************************************************************************************/
    # command to Rate Limiter Cfg table entries
    #/*****************************************************************************************************/
    def do_display_rate_limiter_cfg(self, arg):
        'Display Rate Limiter Cfg entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpsRateLimiterDisplayTable')

    #/*****************************************************************************************************/
    # command to display Pfc Counter table entries
    #/*****************************************************************************************************/
    def do_display_pfc_counter(self, arg):
        'Display Pfc Counter entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpPfcCountersDisplayTable')

    #/*****************************************************************************************************/
    # command to Packet Limit Threshold table entries
    #/*****************************************************************************************************/
    def do_display_pkt_limit_threshold(self, arg):
        'Display Packet Limit Threshold entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpPktLimThresholdDisplayTable')

    #/*****************************************************************************************************/
    # command to display dwrr table entries
    #/*****************************************************************************************************/
    def do_display_dwrr(self, arg):
        'Display DWRR entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpDwrrDisplayTable')

    #/*****************************************************************************************************/
    # command to display copp table entries
    #/*****************************************************************************************************/
    def do_display_copp(self, arg):
        'Display CPU Code entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpsCoppDisplayTable')

    #/*****************************************************************************************************/
    # command to display INT table entries
    #/*****************************************************************************************************/
    def do_display_int(self, arg):
        'Display int entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 6):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                    xpINTDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4], args[5])
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                    xpINTDisplayTable(args[0], numOfValidEntries, args[1], args[2], None,args[3], args[4], 0)
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    xpINTDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0, 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                    xpINTDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, 0, 0, 0)
                elif (len(args) == 2):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                    xpINTDisplayTable(args[0], numOfValidEntries, args[1], args[2], None,0, 0, 0)
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0]))
                    xpINTDisplayTable(args[0], numOfValidEntries, 0, 0, None,0, 0, 0)
                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display egress cos table entries
    #/*****************************************************************************************************/
    def do_display_egress_cos(self, arg):
        'Display egress cos entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpEgressCosMapDisplayTable')

    #/*****************************************************************************************************/
    # command to display shapers table entries
    #/*****************************************************************************************************/
    def do_display_shapers(self, arg):
        'Display Shapers entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpShapersDisplayShapersTable')

    #/*****************************************************************************************************/
    # command to display fast shapers table entries
    #/*****************************************************************************************************/
    def do_display_fast_shapers(self, arg):
        'Display Fast Shapers entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_type_args(args, 'xpShapersDisplayFastShapersTable')

    #/*****************************************************************************************************/
    # command to display insertion table entries
    #/*****************************************************************************************************/
    def do_display_insertion(self, arg):
        'Display Insertion Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpHdrModificationDisplayInsertionTable')

    #/*****************************************************************************************************/
    # command to display IIT table entries
    #/*****************************************************************************************************/
    def do_display_iit(self, arg):
        'Display IIT Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpHdrModificationDisplayIITTable')

    #/*****************************************************************************************************/
    # command to display MIT table entries
    #/*****************************************************************************************************/
    def do_display_mit(self, arg):
        'Display MIT Table entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpHdrModificationDisplayMITTable')

    #/*****************************************************************************************************/
    # command to display tm pipe table entries
    #/*****************************************************************************************************/
    def do_display_tm_pipe(self, arg):
        'Display TM CFG Pipe entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTmCfgDisplayPipeTable')

    #/*****************************************************************************************************/
    # command to display tm port table entries
    #/*****************************************************************************************************/
    def do_display_tm_port(self, arg):
        'Display TM CFG Port entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTmCfgDisplayPortTable')

    #/*****************************************************************************************************/
    # command to display tm h1 table entries
    #/*****************************************************************************************************/
    def do_display_tm_h1(self, arg):
        'Display TM CFG H1 entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTmCfgDisplayH1Table')

    #/*****************************************************************************************************/
    # command to display tm h2 table entries
    #/*****************************************************************************************************/
    def do_display_tm_h2(self, arg):
        'Display TM CFG H2 entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            self.display_table_with_devid_arg(args, 'xpTmCfgDisplayH2Table')

    #/*****************************************************************************************************/
    # command to display h2 counter table entries
    #/*****************************************************************************************************/
    def do_display_h2_counter(self, arg):
        'Display H2 Counter entries, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, Type, [Start-Index, End-Index, detailFormat, silentMode]'
        else:
            self.display_table_with_devid_type_args(args, 'xpH2CounterDisplayTable')

    #/*****************************************************************************************************/
    # command to display Acm Result table entries
    #/*****************************************************************************************************/
    def do_display_acm_result(self, arg):
        'Display Acm Result entries, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id, [Start-Index, End-Index, detailFormat, silentMode]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 5):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d' % (args[0], args[1], args[2], args[3], args[4]))
                    xpAcmRsltDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], args[4])
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0' % (args[0], args[1], args[2], args[3])
)
                    xpAcmRsltDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, args[3], 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0' % (args[0], args[1], args[2]))
                    xpAcmRsltDisplayTable(args[0], numOfValidEntries, args[1], args[2], None, 0, 0)
                elif (len(args) == 2):
                    print('Input Arguments are devId=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0' % (args[0], args[1]))
                    xpAcmRsltDisplayTable(args[0], numOfValidEntries, args[1], 0, None, 0, 0)
                elif (len(args) == 1):
                    print('Input Arguments are devId=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0' % (args[0]))
                    xpAcmRsltDisplayTable(args[0], numOfValidEntries, 0, 0, None, 0, 0)

                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display vif table entries
    #/*****************************************************************************************************/
    def do_display_vif(self, arg):
        'Display Vif entries, Enter Device-Id, VifId, Dir, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter Device-Id, VifId, Dir, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 8):
                    print('Input Arguments are devId=%d, vifId=%d, dir=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], args[6], args[7])
                elif (len(args) == 7):
                    print('Input Arguments are devId=%d, vifId=%d, dir=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], args[6], 0)
                elif (len(args) == 6):
                    print('Input Arguments are devId=%d, vifId=%d, dir=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], 0, 0)
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, vifId=%d, dir=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, 0, 0, 0)
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, vifId=%d, dir=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], 0, None, 0, 0, 0)
                elif (len(args) == 3):
                    print('Input Arguments are devId=%d, vifId%d, dir=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                    xpVifDisplayTable(args[0], args[1], args[2], numOfValidEntries, 0, 0, None, 0, 0, 0)

                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display Acm Bank table entries
    #/*****************************************************************************************************/
    def do_display_acm_bank(self, arg):
        'Display Acm Bank entries, Enter Device-Id, Sde, Client, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, Sde, Client, Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                numOfValidEntries = new_uint32_tp()
                if (len(args) == 9):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], args[8])
                elif (len(args) == 8):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], 0)
                elif (len(args) == 7):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], 0, 0)
                elif (len(args) == 6):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, 0, 0, 0)
                elif (len(args) == 5):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], 0, None, 0, 0, 0)
                elif (len(args) == 4):
                    print('Input Arguments are devId=%d, sde=%d, client=%d, type=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                    xpAcmBankDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, 0, 0, None, 0, 0, 0)

                delete_uint32_tp(numOfValidEntries)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display Iacl table entries
    #/*****************************************************************************************************/
    def do_display_iacl(self, arg):
        'Display Iacl table entries, Enter Device-Id, iaclType, keyType, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 3:
            print 'Invalid input, Enter Device-Id, iaclType, keyType, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            numOfValidEntries = new_uint32_tp()
            if (len(args) == 9):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], args[8])
            elif (len(args) == 8):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], 0)
            elif (len(args) == 7):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], 0, 0)
            elif (len(args) == 6):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, 0, 0, 0)
            elif (len(args) == 5):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], 0, None, 0, 0, 0)
            elif (len(args) == 4):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=%d, startIndex=0, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                xpIaclDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, 0, 0, None, 0, 0, 0)
            elif (len(args) == 3):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, displayEntryFlag=1, startIndex=0, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                xpIaclDisplayTable(args[0], args[1], args[2], 1, numOfValidEntries, 0, 0, None, 0, 0, 0)

            delete_uint32_tp(numOfValidEntries)

    #/*****************************************************************************************************/
    # Validating sequence of IPv4/v6 key-fields
    #/*****************************************************************************************************/
    def validate_keyfields_sequence (self, KeyFieldsList):
        prevkeyfld = 0
        keyValuesinOrder = []
        sortedList = []
        for keyIdx in range(len(KeyFieldsList)):
                if((eval(KeyFieldsList[keyIdx])) < prevkeyfld):
                    print 'Error : Key fields are not in order'
                    print 'Correct order : ' ,
                    for index in range(len(KeyFieldsList)):
                        keyValuesinOrder.append(eval(KeyFieldsList[index]))
                    keyValuesinOrder.sort()
                    for keyIdx in range(len(keyValuesinOrder)):
                         for valIdx in range(len(KeyFieldsList)):
                             if(keyValuesinOrder[keyIdx] == eval(KeyFieldsList[valIdx])):
                                 sortedList.append(KeyFieldsList[valIdx])
                                 break
                    print sortedList
                    return 1
                prevkeyfld = eval(KeyFieldsList[keyIdx])
        return 0

    #/*****************************************************************************************************/
    # command to display Iacl table entries for xpApp
    #/*****************************************************************************************************/
    def do_display_iacl_xp(self, arg):
        '''
         'Display Iacl table entries: Enter devId, iaclType, keyType, numFlds, isValid, fields, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, iaclType, keyType, numFlds, isValid, fields, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example ')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            xpIaclkeyFieldList_t_Ptr.numFlds = args[3]
            xpIaclkeyFieldList_t_Ptr.isValid = args[4]
            xpIaclkeyFieldList_t_Ptr.type = args[2]
            fldList = args[5].strip().split(',')
            if((self.validate_keyfields_sequence(fldList)) != 0):
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                return

            xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
            xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                   if(xpIaclkeyFieldList_t_Ptr.type == 0):
                       xpIaclkeyField_t_ptr.fld.v4Fld = eval(fldEnm)
                       byteSize = mapxpIaclV4KeyByteMask[eval(fldEnm)]
                   else:
                       xpIaclkeyField_t_ptr.fld.v6Fld = eval(fldEnm)
                       byteSize = mapxpIaclV6KeyByteMask[eval(fldEnm)]
                   byteLen = bin(byteSize).count('1')
                   valPtr = new_uint8Arr_tp(byteLen)
                   maskPtr = new_uint8Arr_tp(byteLen)
                   xpIaclkeyField_t_ptr.value = valPtr
                   xpIaclkeyField_t_ptr.mask = maskPtr
                   xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
            xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
            numOfValidEntries = new_uint32_tp()
            #print('Input Arguments are, devId=%d, keyType=%s isValid=%d, numFlds=%d, fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,fldList,valueList,maskList))

            if (len(args) == 12):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6]), int(args[7]), int(args[8]), int(args[9]), int(args[10]), int(args[11])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, int(args[7]), int(args[8]), None, int(args[9]), int(args[10]), int(args[11]))
            elif (len(args) == 11):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6]), int(args[7]), int(args[8]), int(args[9]), int(args[10])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, int(args[7]), int(args[8]), None, int(args[9]), int(args[10]), 0)
            elif (len(args) == 10):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6]), int(args[7]), int(args[8]), int(args[9])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, int(args[7]), int(args[8]), None, int(args[9]), 0, 0)
            elif (len(args) == 9):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6]), int(args[7]), int(args[8])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, int(args[7]), int(args[8]), None, 0, 0, 0)
            elif (len(args) == 8):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=%d, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6]), int(args[7])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, int(args[7]), 0, None, 0, 0, 0)
            elif (len(args) == 7):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=%d, startIndex=0, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], eval(args[6])))
                ret = xpAclDisplayTable(args[0], args[1], eval(args[6]), xpIaclkeyFieldList_t_Ptr, numOfValidEntries, 0, 0, None, 0, 0, 0)
            elif (len(args) == 6):
                print('Input Arguments are devId=%d, iaclType=%d, keyType=%d, numFlds=%d, isValid=%d, fields=%s, displayEntryFlag=1, startIndex=0, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))

                ret = xpAclDisplayTable(args[0], args[1], 1, xpIaclkeyFieldList_t_Ptr, numOfValidEntries, 0, 0, None, 0, 0, 0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            del valPtr
            del maskPtr
            del xpIaclkeyField_t_arr
            delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
            delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            delete_uint32_tp(numOfValidEntries)

    #/*****************************************************************************************************/
    # command to display Eacl table entries
    #/*****************************************************************************************************/
    def do_display_eacl(self, arg):
        'Display Eacl table entries, Enter Device-Id, keyType, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 2:
            print 'Invalid input, Enter Device-Id, keyType, [displayEntryFlag, Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            numOfValidEntries = new_uint32_tp()
            if (len(args) == 8):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], args[6], args[7])
            elif (len(args) == 7):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], args[6], 0)
            elif (len(args) == 6):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, args[5], 0, 0)
            elif (len(args) == 5):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=%d,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], args[4], None, 0, 0, 0)
            elif (len(args) == 4):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=%d, endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, args[3], 0, None, 0, 0, 0)
            elif (len(args) == 3):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=%d, startIndex=0 endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2]))
                xpEaclDisplayTable(args[0], args[1], args[2], numOfValidEntries, 0, 0, None, 0, 0, 0)
            elif (len(args) == 2):
                print('Input Arguments are devId=%d, keyType=%d, displayEntryFlag=0, startIndex=0 endIndex=0,logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1]))
                xpEaclDisplayTable(args[0], args[1], 2, numOfValidEntries, 0, 0, None, 0, 0, 0)

            delete_uint32_tp(numOfValidEntries)

    #/*****************************************************************************************************/
    # command to display Open Flow table entries
    #/*****************************************************************************************************/
    def do_display_open_flow_entries(self, arg):
        'Display Open Flow table entries, Enter Device-Id, DpId, Table-Id, Table-Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        args = tuple(map(int, arg.split()))
        if  len(args) < 4:
            print 'Invalid input, Enter Device-Id, DpId, Table-Id, Table-Type, [Start-Index, End-Index, detailFormat, silentMode, tblCopyIdx]'
        else:
            numOfValidEntries = new_uint32_tp()

            if (len(args) == 9):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=%d' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], args[8])
            elif (len(args) == 8):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=%d, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], args[7], 0)
            elif (len(args) == 7):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=%d, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5], args[6]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, args[6], 0, 0)
            elif (len(args) == 6):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=%d, endIndex=%d, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4], args[5]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], args[5], None, 0, 0, 0)
            elif (len(args) == 5):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=%d, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3], args[4]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, args[4], 0, None, 0, 0, 0)
            elif (len(args) == 4):
                print('Input Arguments are devId=%d, dpId=%d, tableId=%d, tableType=%d, startIndex=0, endIndex=0, logFile=NULL, detailFormat=0, silentMode=0, tblCopyIdx=0' % (args[0], args[1], args[2], args[3]))
                retVal = xpsOfDisplayTable(args[0], args[1], args[2], args[3], numOfValidEntries, 0, 0, None, 0, 0, 0)

                delete_uint32_tp(numOfValidEntries)

            if retVal != 0:
                print('Error returned = %d' % (retVal))

    #/****************************************************************************************************/
    # command to display Eq Port Speed table entries
    #/*****************************************************************************************************/
    def do_display_eq_port_speed(self, arg):
        'Display Eq Port Speed entries, Enter Device-Id'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                baseTblObj = xpDeviceMgr.instance().getDeviceObj(args[0]).getTableObj(XP_TABLE_EQ_DST_PORT_SPEED_COMMON_E,0)
                if baseTblObj is None:
                     print "Error - Table Obj not created"
                else:
                     depth = baseTblObj.getNumOfEntries()
                     width = baseTblObj.getWidth()
                     eqPortObj = getDirectTablePtr(baseTblObj)
                     vect = XpBitVector(width)
                     ret = 0

                     print "===============================+"
                     print "EQ_PORT_SPEED table entries [ deviceId=%d ]|" % (args[0])
                     print "===============================+"
                     print "---------------|---------------+"
                     print "hw             |               |"
                     print "Index          |           data|"
                     print "---------------|---------------+"

                     for index in range(0, depth):
                         vect.reset()
                         ret |= eqPortObj.readEntry(index, vect)
                         print "%15d|%15d|" % (index, vect.toUint32())

                     print "---------------|---------------+"
                     if ret !=0:
                         print "Error returned = %d" % (ret)
            else:
                print ('Device %d not added' %args[0])

    #/****************************************************************************************************/
    # command to display Dq Port Speed table entries
    #/*****************************************************************************************************/
    def do_display_dq_port_speed(self, arg):
        'Display Dq Port Speed entries, Enter Device-Id'
        args = tuple(map(int, arg.split()))
        if  len(args) < 1:
            print 'Invalid input, Enter Device-Id'
        else:
            if (xpDeviceMgr.instance().isDeviceValid(args[0])):
                baseTblObj = xpDeviceMgr.instance().getDeviceObj(args[0]).getTableObj(XP_TABLE_DQ_PORT_SPEED_COMMON_E,0)
                if baseTblObj is None:
                     print "Error - Table Obj not created"
                else:
                     depth = baseTblObj.getNumOfEntries()
                     width = baseTblObj.getWidth()
                     dqPortObj = getDirectTablePtr(baseTblObj)
                     vect = XpBitVector(width)
                     ret = 0

                     print "===============================+"
                     print "DQ_PORT_SPEED table entries [ deviceId=%d ]|" % (args[0])
                     print "===============================+"
                     print "---------------|---------------+"
                     print "hw             |               |"
                     print "Index          |           data|"
                     print "---------------|---------------+"

                     for index in range(0, depth):
                         vect.reset()
                         ret |= dqPortObj.readEntry(index, vect)
                         print "%15d|%15d|" % (index, vect.toUint32())

                     print "---------------|---------------+"
                     if ret !=0:
                         print "Error returned = %d" % (ret)
            else:
                print ('Device %d not added' %args[0])

    #/*****************************************************************************************************/
    # command to display WCM table entries
    #/*****************************************************************************************************/

    def do_display_wcm_iacl(self, arg):
        'Display WCM Iacl table entries, Enter Device-Id, iaclType, keyCount, keyType, keyFlds'
        #args = tuple(map(int, arg.split()))
        args = re.split(';| ',arg)
        if  len(args) < 5:
            print 'Invalid input, Enter Device-Id, iaclType, keyCount, keyType, keyFlds'
        else:
            numOfValidEntries = new_uint32_tp()
            args[0] = int(args[0])
            args[1] = eval(args[1])
            key_Ptr = new_xpWcmIaclkeyFieldList_tp()
            memZero(key_Ptr, sizeof_xpWcmIaclkeyFieldList)
            key_Ptr.numFlds = int(args[2])
            key_Ptr.type = eval(args[3])
            key_Ptr.isValid = 1
            fldList = args[4].strip().split(',')
            xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
            memZero(xpIaclkeyField_t_ptr, sizeof_xpWcmIaclkeyField)
            xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
            memZero(xpIaclkeyField_t_arr, sizeof_xpWcmIaclkeyField * len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                   if(key_Ptr.type == 0):
                       xpIaclkeyField_t_ptr.fld.v4Fld = eval(fldEnm)
                       byteSize =mapxpIaclV4KeyByteMask[eval(fldEnm)]
                       if(xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_L4_SRC_PORT):
                           xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_L4_DEST_PORT):
                           xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_SIP_V4):
                           xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_DIP_V4):
                           xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                       else:
                           xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                   else:
                       xpIaclkeyField_t_ptr.fld.v6Fld = eval(fldEnm)
                       byteSize =mapxpIaclV6KeyByteMask[eval(fldEnm)]
                       if(xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_L4_V6_SRC_PORT):
                           xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v6Fld ==  XP_IACL_L4_V6_DEST_PORT):
                           xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_SIP_V6):
                           xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                       elif (xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_DIP_V6):
                           xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                       else:
                           xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                   byteLen = bin(byteSize).count('1')
                   valPtr = new_uint8Arr_tp(byteLen)
                   maskPtr = new_uint8Arr_tp(byteLen)
                   memZero(valPtr, sizeof_uint8 * byteLen)
                   memZero(maskPtr, sizeof_uint8 * byteLen)
                   xpIaclkeyField_t_ptr.value = valPtr
                   xpIaclkeyField_t_ptr.mask = maskPtr
                   xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
            key_Ptr.fldList = xpIaclkeyField_t_arr
            xpWcmIaclDisplayTable(args[0], args[1], 2, key_Ptr, numOfValidEntries, 0, 0, None, 0, 0, 0)
