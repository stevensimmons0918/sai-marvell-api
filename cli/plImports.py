#!/usr/bin python
#   plImports.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell’s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

import sys
import time
import os
import readline

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
sys.path.append(dirname + "/../cli/pl")

from xpControlMac import *
from xpEgressFilter import *
from xpHdrModification import *
from xpMdt import *
from xpTrunkResolution import *
from xpVif import *
from xpIp4RouteLpm import *
from xpIp6RouteLpm import *
from xpWcmIacl import *

#/*********************************************************************************************************/
# The class object for PL command operations
#/*********************************************************************************************************/
class xpPlOperationCmds(Cmd):

    #/*****************************************************************************************************/
    # sub-commands for PL functions
    #/*****************************************************************************************************/

    def do_pl(self, s):
        ' PL Commands'
        i = xpStaticPlCmds()
        i.prompt = self.prompt[:-1]+':pl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

class xpStaticPlCmds(Cmd):

    #/*****************************************************************************************************/
    # sub-commands for CONTROLMAC functions
    #/*****************************************************************************************************/
    def do_controlmac(self, s):
        ' CONTROLMAC Commands'
        i = xpControlMacObj()
        i.prompt = self.prompt[:-1]+':controlmac)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for EGRESSFILTER functions
    #/*****************************************************************************************************/
    def do_egressfilter(self, s):
        ' EGRESSFILTER Commands'
        i = xpEgressFilterObj()
        i.prompt = self.prompt[:-1]+':egressfilter)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for HDRMODIFICATION functions
    #/*****************************************************************************************************/
    def do_hdrmodification(self, s):
        ' HDRMODIFICATION Commands'
        i = xpHdrModificationObj()
        i.prompt = self.prompt[:-1]+':hdrmodification)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for MDT functions
    #/*****************************************************************************************************/
    def do_mdt(self, s):
        ' MDT Commands'
        i = xpMdtObj()
        i.prompt = self.prompt[:-1]+':mdt)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for TRUNK RESOLUTION functions
    #/*****************************************************************************************************/
    def do_trunk_resolution(self, s):
        ' TRUNK RESOLUTION Commands'
        i = xpTrunkResolutionObj()
        i.prompt = self.prompt[:-1]+':trunkresolution)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for VIF functions
    #/*****************************************************************************************************/
    def do_vif(self, s):
        ' VIF Commands'
        i = xpVifObj()
        i.prompt = self.prompt[:-1]+':vif)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for IPV4ROUTELPM functions
    #/*****************************************************************************************************/
    def do_ipv4routelpm(self, s):
        ' IPV4ROUTELPM Commands'
        i = xpIpv4RouteLpmObj()
        i.prompt = self.prompt[:-1]+':ipv4routelpm)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for IPV6ROUTELPM functions
    #/*****************************************************************************************************/
    def do_ipv6routelpm(self, s):
        ' IPV6ROUTELPM Commands'
        i = xpIpv6RouteLpmObj()
        i.prompt = self.prompt[:-1]+':ipv6routelpm)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for WCM_IACL functions
    #/*****************************************************************************************************/
    def do_wcm_iacl(self, s):
        'WCM_IACL Commands'
        i = xpWcmIaclObj()
        i.prompt = self.prompt[:-1]+':wcm_iacl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    # sub-commands for IPV6ROUTELPM functions
    #/*****************************************************************************************************/
