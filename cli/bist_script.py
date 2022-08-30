#!/usr/bin/env python
# bist_script.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell’s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

import glob
import re
import string
import sys

class bistParsing(object):
    ''' To parse the TDR SIB file for the control bits
    '''
   
    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    def __init__(self):
        self.run_reg = [[] for i in range(12)]
        self.done_pass = [[] for i in range(12)]
        self.done_fail = [[] for i in range(12)]
        self.tdr_end = [125, 461, 5319, 53638, 38, 92983, 259789, 259789, 263063, 287807, 7184, 34239]
    
    #global run_reg
    #global done_pass
    #global done_fail
    #run_reg = [[] for i in range(12)]
    #done_pass = [[] for i in range(12)]
    #done_fail = [[] for i in range(12)]

    def parsing(self):
        
        #fid = open('/scratch/dump1/pd/pd_work_1/mula/trc/VPNR_final/trc_out_1_15/scan_registers_tdr_sib1' ,'r')
        fid = open('./cli/scan_registers_tdr_sib1')
        lines = fid.readlines()
        fid.close()
        prev_TDR = 0

        for line in lines:
            if re.search(r'run_reg', line):
                a = re.search(r'\d{1,2}\[.*\]', line)
                b = a.group()
                c = b.split('[')
                d = c[1].split(']')
                self.run_reg[int(c[0])].append(int(d[0]))
            elif re.search(r'done_pass', line):
                a = re.search(r'\d{1,2}\[.*\]', line)
                b = a.group()
                c = b.split('[')
                d = c[1].split(']')
                self.done_pass[int(c[0])].append(int(d[0]))
            elif re.search(r'done_fail', line):
                a = re.search(r'\d{1,2}\[.*\]', line)
                b = a.group()
                c = b.split('[')
                d = c[1].split(']')
                self.done_fail[int(c[0])].append(int(d[0]))
        
