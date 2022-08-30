#  xpsUtils.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

def invalid_ipv6_address(ipv6addr):
        for i in range(0, len(ipv6addr)):
            if len(ipv6addr[i]) > 2 or len(ipv6addr[i]) < 2:
                return 1
        return 0
    
def invalid_ipv4_address(ipv4addr):
    for i in range(0,len(ipv4addr)):
        if(len(ipv4addr[i]) > 3 or (int(ipv4addr[i]) > 255 or int(ipv4addr[i]) < 0)):
            return 1
    return 0
