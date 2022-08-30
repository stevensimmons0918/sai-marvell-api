/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/



#ifndef __cm3FileCommonh
#define __cm3FileCommonh

extern GT_U32 current_SIM_CM3_DEVICE_ID;/* set as 0 by default */
#define SIM_CM3_DEVICE_ID current_SIM_CM3_DEVICE_ID

GT_STATUS simCm3CurrentDeviceIdSet(IN GT_U32 deviceId/*the deviceIde as in the INI file*/);

/**
* @internal simLogMessage function
* @endinternal
*
* @brief   This routine logs CM3  message.
*
* @retval GT_OK                    - Logging has been done successfully.
* @retval GT_BAD_PARAM             - Wrong parameter.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note Usage example:
*       simCm3LogMessage( cm3DevId,"value is: %d\n", 123);
*
*/
GT_STATUS simCm3LogMessage
(
    IN GT_U32                       cm3DevId,
    IN const GT_CHAR               *formatStringPtr,
    IN ...
);
/*******************************************************************************
* simCm3LogRedirectSet
*
* DESCRIPTION: Whether to redirect CM3 logs to sdtio
*
* INPUT:
*      GT_BOOL redirectLogToStdout - when true CM3 logs are outputed to stdio.
*
* OUTPUT:
*     none
*
*
* RETURN:
*       GT_STATUS - GT_OK
*                   GT_FAIL
*******************************************************************************/

GT_STATUS simCm3LogRedirectSet
(
    GT_BOOL redirectLogToStdout
);
/*******************************************************************************
* simCm3LogRedirectGet
*
* DESCRIPTION: Get redirection mode
*
* INPUT:
*   none
*
*
* OUTPUT:
*     GT_BOOL redirectLogToStdout - when true CM3 logs are outputed to stdio.
*
*
* RETURN:
*       GT_STATUS - GT_OK
*                   GT_FAIL
*******************************************************************************/

GT_BOOL simCm3LogRedirectGet
(
    GT_VOID
);



#define SIM_CM3_LOG(_str,...) \
    do\
    {\
     if(GT_FALSE==simCm3LogRedirectGet())\
     {\
        simCm3LogMessage(SIM_CM3_DEVICE_ID,\
              _str, ##__VA_ARGS__);\
     }\
     else\
     {\
        printf(_str, ##__VA_ARGS__);\
     }\
    }while(0);

#define SIM_CM3_FORCE_PRINT(_str,...) \
    do\
    {\
     /*do printf*/  \
     printf(_str, ##__VA_ARGS__);\
     if(GT_FALSE==simCm3LogRedirectGet())\
     {\
        /*and allow WM log (if enabled)*/   \
        simCm3LogMessage(SIM_CM3_DEVICE_ID,\
              _str, ##__VA_ARGS__);\
     }\
    }while(0);




#define MV_BYTE_SWAP_32BIT(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))


typedef enum {  CM3_FW,         /* Not used */
                EPROM,          /* Register configuration files,  Port/VLAN configuration file */
                script,         /* same like EEPROM */
                AVAGO_FW_sbus,  /* AVAGO_FW_sbus */
                AVAGO_FW_spico, /* AVAGO_FW_spico */
                micro_init,     /* Micro-init FW */
                ignored,
                AVAGO_FW_swap,  /* AVAGO_FW_swap file */
                PHY_FW,         /* PHY firmware */
                PHA_FW,         /* HA data */
                HA_THR_PC,      /* HA addresses */
                OPTIONS,        /* options related to MI */
                DIAG,           /* dual boot application */
                AP_FW           /* AP FW - FALCON ONLY*/
}file_type;


#define HOSTG_IMAGE_STRING_LENGTH   32
#define HOSTG_FILE_NAME_LENGTH       16
#define HOSTG_IMAGE_MAGIC_LENGTH    4
#define HOSTG_DATE_LENGTH           10
#define HOSTG_TIME_LENGTH           8

typedef struct BOOTONP_image_header_STCT
{
   unsigned char            image_magic[HOSTG_IMAGE_MAGIC_LENGTH];
   unsigned short            header_size;
   unsigned short            header_version;
   unsigned int            header_crc;
   unsigned int            total_header_size;
   unsigned int            image_size;
   unsigned int            image_crc;
   unsigned char            image_version[HOSTG_IMAGE_STRING_LENGTH];
   unsigned char            image_time[HOSTG_IMAGE_STRING_LENGTH];
   unsigned int            num_of_files;
}BOOTONP_image_header_STC;

/*
    Files header included:
        Header size             - the file header size.
        Header version          - the header version.
        Header CRC              - only the header crc without the files.
        File size               - specific file image size (header + files).
        File name               - file name, set by "file_list.txt" included when run "build_image.sh" script.
        type                    - file type.
        execution copy offset   - the copy offset from the begining for execution.
        execution offset
        file offset in image    - the offset from the begining of the image file.
        use bmp                 - bitmap
*/
typedef struct BOOTONP_image_file_STCT
{
    unsigned short            header_size;
   unsigned short            header_version;
   unsigned int            header_crc;
   unsigned int            file_size;
    unsigned char            file_name[HOSTG_FILE_NAME_LENGTH];
   unsigned int            type;
   unsigned int            exec_cpy_offset;
    unsigned int            exec_offset;
   unsigned int            image_offset;
   unsigned int            bmp;
}BOOTONP_image_file_STC;

typedef enum
{
    CM3_SIM_PP_FAMILY_DXCH_FALCON_E,
    CM3_SIM_PP_FAMILY_DXCH_AC5P_E,
    CM3_SIM_PP_FAMILY_DXCH_AC5X_E,
    CM3_SIM_PP_FAMILY_DXCH_HARRIER_E
}CM3_SIM_PP_FAMILY_TYPE_ENT;

CM3_SIM_PP_FAMILY_TYPE_ENT microInitCm3HostPpFamilyGet
(
    GT_VOID
);

#endif   /* __cm3FileCommonh */
