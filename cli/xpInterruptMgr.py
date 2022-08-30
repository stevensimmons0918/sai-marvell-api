#!/usr/bin/env python                                                              
#  xpInterruptMgr.py
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
import inspect

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")

from cmd2x import Cmd
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
#/**********************************************************************************/
from buildTarget import *

class xpInterruptMgrObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'
  
    #/********************************************************************************/
    # Auto completion for interrupt enumerations
    #/********************************************************************************/
    def complete_enable_interrupts(self, text, line, begidx, endidx):
        tempDict = { 2 : 'interruptType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/*****************************************************************************************************/
    # command to xpInterruptManagerEnableInterrupt
    #/*****************************************************************************************************/
    def do_enable_interrupts(self, arg):
        '''
        xpInterruptManagerEnableInterrupt: Enter [ devId,interruptType,priority,enable ]'
        Valid values for priority: '1' for High, and '0' for Low
        Valid values for interruptType: <interruptType>
        '''

        args =  arg.split()
        if  len(args) < 3:
            print('Invalid input, Enter [ devId,interruptType,priority,enable ]')
        else:
            #print('Input Arguments are, devId=%d interruptType=%d priority=%d enable=%d' % (int(args[0]), int(args[1]), int(args[2]), int(args[3])))
            ret = xpInterruptManagerEnableInterrupt(int(args[0]), eval(args[1]), int(args[2]), int(args[3]))
            if ret == 0:
                print('Operation Successful!')
            else:
                print('Operation Failed with Error - %d' % ret)
    
    #/********************************************************************************/
    # command for xpInterruptManagerGetInterruptStatistics
    #/********************************************************************************/
    def do_get_interrupt_statistics(self, arg):
        'xpInterruptManagerGetInterruptStatistics: Enter [ devId ]'
    
        args =  arg.split()
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            #print('Input Arguments are, devId=%d' % (int(args[0])))
    
            intr_block_names = ("txq_cnt_stats", "txq_cnt_len",	"txq_tbwrapper", "txq_tbm", "txq_aqm", "txq_eq", "txq_dq",
                                "search_rsltntwk", "search_reqntwk",  "search_age", "search_pooll", "search_poola",
                                "search_poolt", "search_mem", "search_se", "sde1_lde", "sde1_isme", "sde1_urw", "sde1_mme",
                                "sde1_mre", "sde1_hdbf", "sde1_acm", "sde1_parser", "sde0_mem", "sde0_lde", "sde0_isme", "sde0_urw",
                                "sde0_mme",	"sde0_mre",	"sde0_hdbf", "sde0_acm", "sde0_parser",	"dp_bm", "dp_pm",
                                "dp_txdma", "dp_sdma", "dp_rxdma", "dp_ibuffer", "ptg", "dmac", "ptp", "mgmt")
    
            lowPrioIntrCount = new_arrUint64(XP_MAX_INTR_BLOCKS)
            highPrioIntrCount = new_arrUint64(XP_MAX_INTR_BLOCKS)
    
            ret = xpInterruptManagerGetInterruptStatistics(int(args[0]), highPrioIntrCount, lowPrioIntrCount)
            if ret == 0:
                print('---------------------------------------------------------------------------')
                print('****************            Interrupt Statistics            ***************')
                print('---------------------------------------------------------------------------')
                print('  Block Id       Block Name       HighPrio Counter        LowPrio Counter  ')
                print('---------------------------------------------------------------------------')
                for block in range(0, XP_MAX_INTR_BLOCKS):
                    print('%7d%20s%23d%23d' % (block, intr_block_names[block], arrUint64_getitem(highPrioIntrCount, block), arrUint64_getitem(lowPrioIntrCount, block)))
                print('---------------------------------------------------------------------------')
    
            else:
                print('Operation Failed with Error - %d' % ret)
    
            delete_arrUint64(lowPrioIntrCount)
            delete_arrUint64(highPrioIntrCount)

    #/*****************************************************************************************************/
    # command to xpInterruptManagerResetInterruptStatistics
    #/*****************************************************************************************************/
    def do_reset_interrupt_statistics(self, arg):
        'xpInterruptManagerResetInterruptStatistics: Enter [ devId ]'
        args =  arg.split()
        if  len(args) < 1:
            print('Invalid input, Enter [ devId ]')
        else:
            #print('Input Arguments are, devId=%d' % (int(args[0])))
            xpInterruptManagerResetInterruptStatistics(int(args[0]))
            print('Interrupt counters resetted successfully')
