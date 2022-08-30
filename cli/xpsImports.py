#!/usr/bin python
#  pkgImports.py
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
import readline

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + ".")
sys.path.append(dirname + "./xps")
sys.path.append(dirname + "./../cli")
sys.path.append(dirname + "./../cli/features")
sys.path.append(dirname + "./../../cli")
sys.path.append(dirname + "./../../cli/features")
sys.path.append(dirname + "/xps")
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../cli/features")
sys.path.append(dirname + "/../../cli")
sys.path.append(dirname + "/../../cli/features")

if (("saiShell" not in sys.modules.keys())):
    pass

from cmd2x import Cmd
from buildTarget import *
from regAccess import *

from xpsEacl import *
from xpsIpGre import *
from xpsIpinIp import *
from xpsGeneve import *
from xpsVxlan import *
from xpsNvgre import *
from xpsIacl import *
from xpsInit import *
from xpsMac import *
from xpsPolicer import *
from xpsVlan import *
from xpsAcm import *
from xpsInterface import *
from xpsMirror import *
from xpsPort import *
from xpsSerdes import *
from xpsInternal import *
from xpsCommon import *
from xpsPbb import *
from xpsMpls import *
from xpsSflow import *
from xpsEnums import *
from xpsL3 import *
from xpsMulticast import *
from xpsQos import *
from xpsFdb import *
from xpsLag import *
from xpsNat import *
from xpsStp import *
from xpsLink import *
from xpsPacketDrv import *
from xpsTunnel import *
from xpsEgressFilter import *
from xpsErspanGre import *
from xpsAging import *
from xpsGlobalSwitchControl import *
from xps8021Br import *
from xpsNhGrp import *
from xpsMtuProfile import *
from xpsVpnGre import *
from xpsPtp import *
from xpsScope import *
from xpsCopp import*
from xpsWcmIacl import *
from xpsPacketTrakker import *
from xpsSr import *
from xpsInt import*

#/*********************************************************************************************************/
# The class object for XPS Layer operations
#/*********************************************************************************************************/
class xpsLayerOperationCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'
    try :
        profileType = XP_DEFAULT_TWO_PIPE_PROFILE
    except:
        profileType = 1 #XP_DEFAULT_TWO_PIPE_PROFILE


    #/*****************************************************************************************************/
    # sub-commands for XPS layer EACL functions
    #/*****************************************************************************************************/
    def acl(self, s):
        'XPS EACL Commands'
        i = xpsEaclObj()
        i.prompt = self.prompt[:-1]+':acl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer EACL functions
    #/*****************************************************************************************************/
    def do_br(self, s):
        'XPS BR Commands'
        i = xps8021BrObj()
        i.prompt = self.prompt[:-1]+':br)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer INIT functions
    #/*****************************************************************************************************/
    def do_init(self, s):
        'XPS INIT Commands'
        i = xpsInitObj()
        i.prompt = self.prompt[:-1]+':init)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer MAC functions
    #/*****************************************************************************************************/
    def do_mac(self, s):
        'XPS MAC Commands'
        i = xpsMacObj()
        i.prompt = self.prompt[:-1]+':mac)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer MTU profile functions
    #/*****************************************************************************************************/
    def do_mtuprofile(self, s):
        'XPS Mtu profile Commands'
        i = xpsMtuProfileObj()
        i.prompt = self.prompt[:-1]+':mtu)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer POLICER functions
    #/*****************************************************************************************************/
    def do_policer(self, s):
        'XPS POLICER Commands'
        i = xpsPolicerObj()
        i.prompt = self.prompt[:-1]+':policer)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer VLAN functions
    #/*****************************************************************************************************/
    def do_vlan(self, s):
        'XPS VLAN Commands'
        i = xpsVlanObj()
        i.prompt = self.prompt[:-1]+':vlan)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer VPN-GRE functions
    #/*****************************************************************************************************/
    def do_vpngre(self, s):
        'XPS VPN-GRE Commands'
        i = xpsVpnGreObj()
        i.prompt = self.prompt[:-1]+':vpngre)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer ACM functions
    #/*****************************************************************************************************/
    def do_acm(self, s):
        'XPS ACM Commands'
        i = xpsAcmObj()
        i.prompt = self.prompt[:-1]+':acm)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer INTERFACE functions
    #/*****************************************************************************************************/
    def do_interface(self, s):
        'XPS INTERFACE Commands'
        i = xpsInterfaceObj()
        i.prompt = self.prompt[:-1]+':interface)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer MIRROR functions
    #/*****************************************************************************************************/
    def do_mirror(self, s):
        'XPS MIRROR Commands'
        i = xpsMirrorObj()
        i.prompt = self.prompt[:-1]+':mirror)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer PORT functions
    #/*****************************************************************************************************/
    def do_port(self, s):
        'XPS PORT Commands'
        i = xpsPortObj()
        i.prompt = self.prompt[:-1]+':port)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer SERDES functions
    #/*****************************************************************************************************/
    def do_serdes(self, s):
        'XPS SERDES Commands'
        i = xpsSerdesObj()
        i.prompt = self.prompt[:-1]+':serdes)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer INTERNAL functions
    #/*****************************************************************************************************/
    def do_internal(self, s):
        'XPS INTERNAL Commands'
        i = xpsInternalObj()
        i.prompt = self.prompt[:-1]+':internal)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer COMMON functions
    #/*****************************************************************************************************/
    def do_common(self, s):
        'XPS COMMON Commands'
        i = xpsCommonObj()
        i.prompt = self.prompt[:-1]+':common)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer MPLS functions
    #/*****************************************************************************************************/
    def do_mpls(self, s):
        'XPS MPLS Commands'
        i = xpsMplsObj()
        i.prompt = self.prompt[:-1]+':mpls)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer SFLOW functions
    #/*****************************************************************************************************/
    def do_sflow(self, s):
        'XPS SFLOW Commands'
        i = xpsSflowObj()
        i.prompt = self.prompt[:-1]+':sflow)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer ENUMS functions
    #/*****************************************************************************************************/
    def enums(self, s):
        'XPS ENUMS Commands'
        i = xpsEnumsObj()
        i.prompt = self.prompt[:-1]+':enums)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer L3 functions
    #/*****************************************************************************************************/
    def do_l3(self, s):
        'XPS L3 Commands'
        i = xpsL3Obj()
        i.prompt = self.prompt[:-1]+':l3)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer NhGrp functions
    #/*****************************************************************************************************/
    def do_nhgrp(self, s):
        'XPS NhGrp Commands'
        i = xpsNhGrpObj()
        i.prompt = self.prompt[:-1]+':nhgrp)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer MULTICAST functions
    #/*****************************************************************************************************/
    def do_multicast(self, s):
        'XPS MULTICAST Commands'
        i = xpsMulticastObj()
        i.prompt = self.prompt[:-1]+':multicast)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer QOS functions
    #/*****************************************************************************************************/
    def do_qos(self, s):
        'XPS QOS Commands'
        i = xpsQosObj()
        i.prompt = self.prompt[:-1]+':qos)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer FDB functions
    #/*****************************************************************************************************/
    def do_fdb(self, s):
        'XPS FDB Commands'
        i = xpsFdbObj()
        i.prompt = self.prompt[:-1]+':fdb)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer PBB functions
    #/*****************************************************************************************************/
    def do_pbb(self, s):
        'XPS PBB Commands'
        i = xpsPbbObj()
        i.prompt = self.prompt[:-1]+':pbb)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer LAG functions
    #/*****************************************************************************************************/
    def do_lag(self, s):
        'XPS LAG Commands'
        i = xpsLagObj()
        i.prompt = self.prompt[:-1]+':lag)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer NAT functions
    #/*****************************************************************************************************/
    def do_nat(self, s):
        'XPS NAT Commands'
        i = xpsNatObj()
        i.prompt = self.prompt[:-1]+':nat)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer STP functions
    #/*****************************************************************************************************/
    def do_stp(self, s):
        'XPS STP Commands'
        i = xpsStpObj()
        i.prompt = self.prompt[:-1]+':stp)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer LINK functions
    #/*****************************************************************************************************/
    def do_link(self, s):
        'XPS LINK Commands'
        i = xpsLinkObj()
        i.prompt = self.prompt[:-1]+':link)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer PACKETDRV functions
    #/*****************************************************************************************************/
    def do_packetdrv(self, s):
        'XPS PACKETDRV Commands'
        i = xpsPacketDrvObj()
        i.prompt = self.prompt[:-1]+':packetdrv)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer TUNNEL functions
    #/*****************************************************************************************************/
    def do_tunnel(self, s):
        'XPS TUNNEL Commands'
        i = xpsTunnelObj()
        i.prompt = self.prompt[:-1]+':tunnel)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer EACL functions
    #/*****************************************************************************************************/
    def do_eacl(self, s):
        'XPS EACL Commands'
        i = xpsEaclObj()
        i.prompt = self.prompt[:-1]+':eacl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer IPGRE functions
    #/*****************************************************************************************************/
    def do_ipgre(self, s):
        'XPS IPGRE Commands'
        i = xpsIpGreObj()
        i.prompt = self.prompt[:-1]+':ipgre)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer IPINIP functions
    #/*****************************************************************************************************/
    def do_ipinip(self, s):
        'XPS IPINIP Commands'
        i = xpsIpinIpObj()
        i.prompt = self.prompt[:-1]+':ipinip)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer GENEVE functions
    #/*****************************************************************************************************/
    def do_geneve(self, s):
        'XPS GENEVE Commands'
        i = xpsGeneveObj()
        i.prompt = self.prompt[:-1]+':geneve)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer VXLAN functions
    #/*****************************************************************************************************/
    def do_vxlan(self, s):
        'XPS VXLAN Commands'
        i = xpsVxlanObj()
        i.prompt = self.prompt[:-1]+':vxlan)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer NVGRE functions
    #/*****************************************************************************************************/
    def do_nvgre(self, s):
        'XPS NVGRE Commands'
        i = xpsNvgreObj()
        i.prompt = self.prompt[:-1]+':nvgre)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer IACL functions
    #/*****************************************************************************************************/
    def do_iacl(self, s):
        'XPS IACL Commands'
        if(self.profileType == XP_ACL_WCM_PROFILE):
            print "XPS IACL is not supports for profile type 'XP_ACL_WCM_PROFILE'"
        else:
            i = xpsIaclObj()
            i.prompt = self.prompt[:-1]+':iacl)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid inputs format, check help <command> or ? for usage'
                    continue
            del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer Egress Filter functions
    #/*****************************************************************************************************/
    def do_egressflt(self, s):
        'XPS Egress Filter Commands'
        i = xpsEgressFilterObj()
        i.prompt = self.prompt[:-1]+':egress filter)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    # sub-commands for XPS layer ERSPAN functions
    #/*****************************************************************************************************/
    def do_erspan(self, s):
        'XPS ERSPAN Commands'
        i = xpsErspanGreObj()
        i.prompt = self.prompt[:-1]+':erspan)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer  Aging functions
    #/*****************************************************************************************************/
    def do_aging(self, s):
        'XPS Aging Commands'
        i = xpsAgingObj()
        i.prompt = self.prompt[:-1]+':Aging)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer  Aging functions
    #/*****************************************************************************************************/
    def do_global_switch_control(self, s):
        'XPS Global Switch Control Commands'
        i = xpsGlobalSwitchControlObj()
        i.prompt = self.prompt[:-1]+':GlobalSwitchControl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer SCOPE functions
    #/*****************************************************************************************************/
    def do_scope(self, s):
        'XPS SCOPE Commands'
        i = xpsScopeObj()
        i.prompt = self.prompt[:-1]+':scope)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer PTP  functions
    #/*****************************************************************************************************/
    def do_ptp(self, s):
        'XPS Ptp Commands'
        i = xpsPtpObj()
        i.prompt = self.prompt[:-1]+':Ptp)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer COPP functions
    #/*****************************************************************************************************/
    def do_copp(self, s):
        'XPS COPP Commands'
        i = xpsCoppObj()
        i.prompt = self.prompt[:-1]+':copp)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for XPS layer INT functions
    #/*****************************************************************************************************/
    def do_int(self, s):
        'XPS INT Commands'
        i = xpsIntObj()
        i.prompt = self.prompt[:-1]+':int)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

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

    #/*****************************************************************************************************/
    # sub-commands for XPS layer WCM IACL functions
    #/*****************************************************************************************************/
    def do_wcm_iacl(self, s):
        'XPS WCM IACL Commands'

        if(self.profileType == XP_ACL_WCM_PROFILE):
            i = xpsWcmIaclObj()
            i.prompt = self.prompt[:-1]+':wcm_iacl)'
            while True:
                try:
                    i.cmdloop()
                    break
                except (ValueError, IndexError), e:
                    print 'Invalid inputs format, check help <command> or ? for usage'
                    continue
            del i
        else:
            print "XPS WCM IACL is supports only for profile type 'XP_ACL_WCM_PROFILE'"

 #/*****************************************************************************************************/
    # sub-commands for XPS layer PACKETTRAKKER functions
    #/*****************************************************************************************************/
    def do_packettrakker(self, s):
        'XPS PACKETTRAKKER Commands'
        i = xpsPacketTrakkerObj()
        i.prompt = self.prompt[:-1]+':packettrakker)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for XPS layer SR functions
    #/*****************************************************************************************************/
    def do_segment_route(self, s):
        'XPS Segment route Commands'
        i = xpsSrObj()
        i.prompt = self.prompt[:-1]+':segment_route)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i
