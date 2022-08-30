/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef INCLUDE_MPD
    #ifdef MPD
    #define INCLUDE_MPD
    #endif /*MPD*/
#endif /*!INCLUDE_MPD*/

#include <lua.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>

#include "pdl/init/pdlInit.h"
#include "pdl/lib/pdlLib.h"
#include "pdl/button/pdlBtn.h"
#include <pdl/sensor/pdlSensor.h>
#include <pdl/fan/pdlFan.h>
#include <iDbgPdl/init/iDbgPdlInit.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/phy/pdlPhy.h>
#include <pdl/sfp/pdlSfp.h>
#ifdef INCLUDE_MPD
    #include <mpd.h>
    #include <mpdPrv.h>
#endif /*INCLUDE_MPD*/

/* indication that we (currently) NOT support opening of *.tar.gz files */
#define IS_SUPPORT_ARCHIVE_FILES   0

#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#if IS_SUPPORT_ARCHIVE_FILES == 1
    #include <archive.h>
    #include <archive_entry.h>
#endif /*IS_SUPPORT_ARCHIVE_FILES*/
#include <fcntl.h>
#include <unistd.h>
int usleep(int usec);/*needed for usleep*/
#endif

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SEmbedded/simFS.h>
#endif /*ASIC_SIMULATION*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef INCLUDE_MPD
typedef struct {
        MPD_TYPE_ENT         phy_type;
        char                    * main_file_name;
        char                    * slave_file_name;
}util_phy_firmware_files_STC;

static util_phy_firmware_files_STC util_phy_firmware_file_names[] = {
        {MPD_TYPE_88X32x0_E, "32x0/phycode_3220.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X33x0_E, "33x0/phycode_3340.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X20x0_E, "20x0/phycode_2040.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X2180_E, "2180/phycode_2180.hdr","slave_download_file/flashdownloadlslave.bin"}
};

#define util_phy_firmware_files_size_CNS        (sizeof(util_phy_firmware_file_names) / sizeof (util_phy_firmware_files_STC))
#endif

/*************** UTILS ************************/

/*****************************************************************************
* FUNCTION NAME: util_cm_register_read
*
* DESCRIPTION:          implementation of c&m register read for pdl
*
*
*****************************************************************************/
static GT_STATUS         util_cm_register_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
        GT_U32                        data;
    GT_STATUS       status;
        status = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);
        data &= mask;
        *dataPtr = data;
        return status;
}

/*****************************************************************************
* FUNCTION NAME: util_cm_register_write
*
* DESCRIPTION:          implementation of c&m register write for pdl
*
*
*****************************************************************************/
static GT_STATUS         util_cm_register_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
        GT_U32                        oldValue, newValue;
        GT_STATUS       status;
        status = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &oldValue);
        if (status == GT_OK){
        newValue = (~mask & oldValue) | (mask  & data);
        status = cpssDrvPpHwInternalPciRegWrite (devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, newValue);
        }
    return status;
}

/*****************************************************************************
* FUNCTION NAME: util_register_read
*
* DESCRIPTION:          implementation of register read for pdl
*
*
*****************************************************************************/
static GT_STATUS         util_register_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
        GT_U32                        data;
        GT_STATUS       status;
        status = cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);
        data &= mask;
        *dataPtr = data;
        return status;
}

/*****************************************************************************
* FUNCTION NAME: util_register_write
*
* DESCRIPTION:          implementation of register write for pdl
*
*
*****************************************************************************/
static GT_STATUS         util_register_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
        GT_U32                        oldValue, newValue;
        GT_STATUS       status;
        status = cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &oldValue);
        if (status == GT_OK){
            newValue = (~mask & oldValue) | (mask  & data);
            status = cpssDrvPpHwRegisterWrite (devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, newValue);
        }
    return status;
}

/*****************************************************************************
* FUNCTION NAME: util_malloc
*
* DESCRIPTION:         implementation of malloc for pdl
*
*
*****************************************************************************/
static void * util_malloc
(
        SIZE_T                size
)
{
        return malloc ((size_t) size);
}

#if 0
/*****************************************************************************
* FUNCTION NAME: util_malloc
*
* DESCRIPTION:         implementation of malloc for pdl
*
*
*****************************************************************************/
static void * util_malloc_2
(
        UINT_32               size
)
{
        return malloc ((size_t) size);
}
#endif
/*****************************************************************************
* FUNCTION NAME: util_smi_read
*
* DESCRIPTION:         implementation of smi read for pdl
*
*
*****************************************************************************/

static GT_STATUS util_smi_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
        IN  GT_U8       phyPageSelReg,
        IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    OUT GT_U16     *dataPtr
)
{
        GT_U16          old_page=0;
        GT_STATUS rc;
        /* TODO: PPU disable before & enable after */
        rc = cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, &old_page);
        rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage);
        rc |= cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr);
        rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, old_page);
        return rc;
}

/*****************************************************************************
* FUNCTION NAME: util_smi_write
*
* DESCRIPTION:         implementation of smi write for pdl
*
*
*****************************************************************************/
static GT_STATUS util_smi_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
        IN  GT_U8       phyPageSelReg,
        IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    IN  GT_U16      data
)
{
        GT_U16          old_page=0;
        GT_STATUS rc;
        /* TODO: PPU disable before & enable after */
        rc = cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, &old_page);
        rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage);
        rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data);
        rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, old_page);

        return rc;
}

#if (defined LINUX && IS_SUPPORT_ARCHIVE_FILES == 1)
static int
    copy_data(struct archive *from, struct archive *to)
{
    int r;
    const void *buff;
    size_t size;
    int64_t offset;

    for (;;) {
        r = archive_read_data_block(from, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK) {
            cpssOsPrintf("%s\n", archive_error_string(from));
            return (r);
        }
        r = archive_write_data_block(to, buff, size, offset);
        if (r != ARCHIVE_OK) {
            cpssOsPrintf("%s\n", archive_error_string(from));
            return (r);
        }
    }
}
#endif

static BOOLEAN util_xml_verification(
    IN  char       *xmlFileNamePtr,
    IN  char       *signatureFileNamePtr
)
{
    PDL_UNUSED_PARAM(xmlFileNamePtr);
    PDL_UNUSED_PARAM(signatureFileNamePtr);
        return TRUE;
}

/*****************************************************************************
* FUNCTION NAME: util_archive_uncompress
*
* DESCRIPTION:  Uncompress xml and md5 files from archive.
*
*
*****************************************************************************/
static BOOLEAN util_archive_uncompress(
    IN  char  * ar_filename_PTR,
    OUT char * xml_filename_PTR,
    OUT char * signature_filename_PTR        /* ignored */
)
{
#if (defined LINUX && IS_SUPPORT_ARCHIVE_FILES == 1)
    struct archive         *in_archive_PTR;
    struct archive         *out_archive_PTR;
    struct archive_entry   *entry_PTR;
    char                    dirname[512];
    int                     res;
#endif
    BOOLEAN                 ret_code = TRUE;
    char                   *tmpStr;
    FILE                   *xmlFilePtr;
    if (ar_filename_PTR == NULL || xml_filename_PTR == NULL)
    {
        cpssOsPrintf("No filename string was supplied\n");
        return FALSE;
    }

#ifdef ASIC_SIMULATION
    cpssOsPrintf("looking for file: [%s %s] \n",
        simFSiniFileDirectory,
        ar_filename_PTR);
#endif /*ASIC_SIMULATION*/

    xml_filename_PTR[0] = '\0';
    /* already an XML file */
    if (strstr(ar_filename_PTR, ".xml") != NULL) {
        strcpy(xml_filename_PTR, ar_filename_PTR);
        strcpy(signature_filename_PTR, ar_filename_PTR);
        tmpStr = strstr(signature_filename_PTR, ".xml");
        if (!tmpStr)
        {
            cpssOsPrintf("Can't change md5 filename string content\n");
            return FALSE;
        }
        cpssOsSprintf(tmpStr, ".md5");
        xmlFilePtr = fopen(xml_filename_PTR, "rb");
        if (!xmlFilePtr)
        {
            cpssOsPrintf("File [%s] not found \n",xml_filename_PTR);
            return FALSE;
        }
        fclose(xmlFilePtr);
        return TRUE;
    }
#if (defined LINUX && IS_SUPPORT_ARCHIVE_FILES == 1)
    in_archive_PTR = archive_read_new();
    out_archive_PTR = archive_write_disk_new();
    if (in_archive_PTR == NULL || out_archive_PTR == NULL)
    {
        if (in_archive_PTR)
            archive_read_free(in_archive_PTR);
        if (out_archive_PTR)
            archive_write_free(out_archive_PTR);
        cpssOsPrintf("Uncompressing failed due to lack of resources!\n");
        return FALSE;
    }
    archive_write_disk_set_options(out_archive_PTR, ARCHIVE_EXTRACT_TIME);
    archive_read_support_filter_gzip(in_archive_PTR);
    archive_read_support_format_tar(in_archive_PTR);
    if ((res = archive_read_open_filename(in_archive_PTR, ar_filename_PTR, 10240)))
    {
        cpssOsPrintf("%s\n", archive_error_string(in_archive_PTR));
        archive_read_free(in_archive_PTR);
        archive_write_free(out_archive_PTR);
        return FALSE;
    }

    for (;;) {
        res = archive_read_next_header(in_archive_PTR, &entry_PTR);
        if (res == ARCHIVE_EOF)
            break;

        if (res != ARCHIVE_OK) {
            cpssOsPrintf("%s\n", archive_error_string(in_archive_PTR));
            ret_code = FALSE;
            break;
        }
        /* change directory of result file */
        cpssOsSprintf(dirname, "%s", ar_filename_PTR);
        tmpStr = strstr(dirname, ".tar.gz");
        if (tmpStr)
            *tmpStr = '\0';
        strcat(dirname, "/");
        strcat(dirname, archive_entry_pathname(entry_PTR));
        archive_entry_set_pathname(entry_PTR, dirname);
        res = archive_write_header(out_archive_PTR, entry_PTR);
        if (res != ARCHIVE_OK) {
            cpssOsPrintf("%s\n", archive_error_string(in_archive_PTR));
            ret_code = FALSE;
            break;
        }
        else {
            res = copy_data(in_archive_PTR, out_archive_PTR);
            if (res != ARCHIVE_OK)
            {
                cpssOsPrintf("Uncompressing failed due to bad archive!\n");
                ret_code = FALSE;
                break;
            }
        }
    }

    archive_read_close(in_archive_PTR);
    archive_read_free(in_archive_PTR);
    archive_write_free(out_archive_PTR);
        /* the files are in directory - add its name */
        strcpy(xml_filename_PTR, ar_filename_PTR);
        tmpStr = strstr(xml_filename_PTR, ".tar.gz");
        if (tmpStr)
                *tmpStr = '\0';
     strcat(xml_filename_PTR, "/");
#endif
    /* verify xml file existence */
    tmpStr = strstr(ar_filename_PTR, ".tar.gz");
    while (tmpStr && tmpStr != ar_filename_PTR)
    {
        tmpStr --;
        if (*tmpStr == '/' || *tmpStr == '\\')
        {
            strcat(xml_filename_PTR, (char *)(tmpStr+1));
            break;
        }
    }
     if (!tmpStr || tmpStr == ar_filename_PTR)
        strcat(xml_filename_PTR, ar_filename_PTR);
    tmpStr = strstr(xml_filename_PTR, ".tar.gz");
    if (!tmpStr)
    {
        cpssOsPrintf("Can't change string content\n");
        return FALSE;
    }

    cpssOsSprintf(tmpStr, ".xml");
    xmlFilePtr = fopen(xml_filename_PTR, "rb");
    if (!xmlFilePtr)
    {
        cpssOsPrintf("Uncompressing failed. No xml file was created!\n");
        return FALSE;
    }
    fclose(xmlFilePtr);

    return ret_code;
}


#ifdef INCLUDE_MPD
/*****************************************************************************
* FUNCTION NAME: util_sleep
*
* DESCRIPTION:         implementation of sleep in ms for MPD
*
*
*****************************************************************************/
static BOOLEAN util_sleep
(
        GT_U32                time_in_ms
)
{
        cpssOsTimerWkAfter(time_in_ms);
        return TRUE;
}

/*****************************************************************************
* FUNCTION NAME: util_phy_transceiver_enable
*
* DESCRIPTION:         implementation of transceiver for MPD
*
*
*****************************************************************************/
static BOOLEAN util_phy_transceiver_enable
(
        GT_U32      rel_ifIndex,
        BOOLEAN     enable
)
{
        GT_U32                      max_retries = 3, count = 0;
        GT_U8                       dev, port;
        PDL_STATUS                  pdl_status;
        PDL_SFP_TX_ENT              pdl_read_value = PDL_SFP_TX_DISABLE_E, pdl_write_value;
        PRV_MPD_PORT_HASH_ENTRY_STC *port_entry_PTR = NULL;
        port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        if (port_entry_PTR == NULL) {
                return FALSE;
        }
        dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
        port = port_entry_PTR->initData_PTR->port;

        pdl_write_value = enable ? PDL_SFP_TX_ENABLE_E : PDL_SFP_TX_DISABLE_E;
        pdl_read_value = enable ? PDL_SFP_TX_DISABLE_E : PDL_SFP_TX_ENABLE_E;

        while (pdl_read_value != pdl_write_value)
        {
                if (count >= max_retries)
                        break;

                pdl_status = pdlSfpHwTxSet (dev, port, pdl_write_value);
                if (pdl_status != PDL_OK) {
                        return FALSE;
                }

                util_sleep(100); /* sleep in-order to give the peripheral to finish the transaction */

                pdl_status = pdlSfpHwTxGet(dev, port, &pdl_read_value);
                if (pdl_status != PDL_OK) {
                        return FALSE;
                }

                count++;
        }

        return TRUE;

}

/*****************************************************************************
* FUNCTION NAME: util_phy_handle_failure
*
* DESCRIPTION:         implementation of handle failure for MPD
*
*
*****************************************************************************/
static MPD_RESULT_ENT util_phy_handle_failure
(
        UINT_32                   rel_ifIndex,
        MPD_ERROR_SEVERITY_ENT    severity,
        UINT_32                   line,
        const char              * calling_func_PTR,
        const char                                * error_text_PTR
)
{
    PDL_UNUSED_PARAM(rel_ifIndex);
    PDL_UNUSED_PARAM(severity);

        cpssOsPrintf("MPDError - line: %1d func: %2s msg: %3s\n", line, calling_func_PTR, error_text_PTR);
        return MPD_OK_E;
}

/*****************************************************************************
* FUNCTION NAME: util_phy_file_load
*
* DESCRIPTION:         implementation of Load a file
*
*
*****************************************************************************/
static void util_phy_file_load
(
        char    *codeFileName_PTR,
        void    *mem_PTR
)
{

        FILE        * filePTR;
        size_t        fileSize;
        size_t        actuallyRead;


        if ( (codeFileName_PTR == NULL) || (mem_PTR == NULL) )
                return;

        if ( (filePTR = fopen(codeFileName_PTR, "rb")) != NULL )
        {
                /* Calculate the file length */
                fseek(filePTR, 0, SEEK_END);
                fileSize = ftell(filePTR);
                fseek(filePTR, 0, SEEK_SET);

                actuallyRead = fread((void *)mem_PTR, 1, fileSize, filePTR);
                if(actuallyRead == 0xFFFFFFFF)
                {
                    cpssOsPrintf("");
                }

                fclose(filePTR);
        }
}

/*****************************************************************************
* FUNCTION NAME: util_phy_file_get_size
*
* DESCRIPTION:         implementation of get packed file size of phy FW
*
*
*****************************************************************************/
static UINT_32 util_phy_file_get_size
(
        char          *codeFileName_PTR      /* File name */
)
{

        FILE * filePTR;
        UINT_32 fileSize;


        if ( codeFileName_PTR == NULL )
                return 0;

        /* Open file stream */
        if ( (filePTR = fopen(codeFileName_PTR, "rb")) == NULL )
                return (0);

        /* Calculate the file length */
        fseek(filePTR, 0, SEEK_END);
        fileSize = ftell(filePTR);
        fclose(filePTR);

        return fileSize;
}

/*****************************************************************************
* FUNCTION NAME: util_phy_get_fw_files
*
* DESCRIPTION:         implementation of handle get FW files for MPD
*
*
*****************************************************************************/
static BOOLEAN util_phy_get_fw_files
(
        IN MPD_TYPE_ENT          phyType,
        OUT MPD_FW_FILE_STC        * mainFile_PTR
)
{
        UINT_32                                                          i;
        util_phy_firmware_files_STC            * file_names_PTR;
        char                                                          main_file_name[256];
        char                                                          slave_file_name[256];

        if (phyType >= MPD_TYPE_NUM_OF_TYPES_E)
                return FALSE;

        for (i = 0; i < util_phy_firmware_files_size_CNS; i++) {
                if (util_phy_firmware_file_names[i].phy_type == phyType) {
                        /* found */
                        break;
                }
        }
        if (i >= util_phy_firmware_files_size_CNS) {
                return FALSE;
        }
        file_names_PTR = &util_phy_firmware_file_names[i];

#ifdef ASIC_SIMULATION
        cpssOsSprintf (main_file_name,  "./phy_firmware/%s", file_names_PTR->main_file_name);
        cpssOsSprintf (slave_file_name, "./phy_firmware/%s", file_names_PTR->slave_file_name);
#else
        cpssOsSprintf (main_file_name,  "/usr/phy_firmware/%s", file_names_PTR->main_file_name);
        cpssOsSprintf (slave_file_name, "/usr/phy_firmware/%s", file_names_PTR->slave_file_name);
#endif

        /* MAIN FW file handling */
        if (main_file_name == NULL)
                return FALSE;

        /* get firmware file size */
        mainFile_PTR->dataSize = util_phy_file_get_size(main_file_name);
        if (mainFile_PTR->dataSize == 0) {
                mainFile_PTR->data_PTR = NULL;
                return FALSE;
        }

        /* allocate space for firmware file to load into ram */
        mainFile_PTR->data_PTR = PRV_MPD_ALLOC_MAC(mainFile_PTR->dataSize);
        if (mainFile_PTR->data_PTR == NULL) {
                return FALSE;
        }

        /* load firmware file into ram */
        util_phy_file_load(main_file_name, mainFile_PTR->data_PTR);

        return TRUE;
}

/*****************************************************************************
* FUNCTION NAME: utils_pdl_phy_init_ports_info
*
* DESCRIPTION:         init phy ports from pdl
*
*
*****************************************************************************/
static PDL_STATUS utils_pdl_phy_init_ports_info
(
)
{
        UINT_32                                   dev, port, nextDev, nextPort, rel_ifIndex;
        PDL_STATUS                                status;
        MPD_PORT_INIT_DB_STC                              phy_entry;
#if 0
        PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC    portAttributes;
        PDL_PHY_CONFIGURATION_STC                 phy_configuration;
        BOOLEAN                                                                          phy_representative_exists[20]={0};
#endif
        status = pdlPpDbFirstPortAttributesGet(&dev, &port);
        rel_ifIndex = 1;
        while (status == PDL_OK)
        {
                memset(&phy_entry, 0, sizeof(phy_entry));
                phy_entry.mdioInfo.mdioDev = (UINT_8) dev;
                phy_entry.port = (UINT_8) port;
#if 0
                if ( (pdlPpDbPortAttributesGet(dev, port, &portAttributes) != PDL_OK) || (portAttributes.isPhyExists == FALSE) )
                {
                        phy_entry.mdioType = MPD_MDIO_TYPE_OTHER_E;
                }
                else
                {
                        PdlPhyDbPhyConfigurationGet(portAttributes.phyData.phyNumber, &phy_configuration);
                        switch (phy_configuration.phyType)
                        {
                                case PDL_PHY_TYPE_alaska_88E1543_E:
                                        /* TBD -
                                        phy_entry.mdioInfo.smi.internalSmiAddress = secondary_phy_addr; */
                                        break;
                                case PDL_PHY_TYPE_alaska_88E1548_E:
                                case PDL_PHY_TYPE_alaska_88E1545_E:
                                case PDL_PHY_TYPE_alaska_88E1680_E:
                                case PDL_PHY_TYPE_alaska_88E1680L_E:
                                case PDL_PHY_TYPE_alaska_88E3680_E:
                                        phy_entry.mdioType = MPD_MDIO_TYPE_SMI_E;
                                        phy_entry.mdioInfo.smi.smiInterface = portAttributes.phyData.smiXmsiInterface.interfaceId;
                                        phy_entry.mdioInfo.smi.smiDev = portAttributes.phyData.smiXmsiInterface.dev;
                                        phy_entry.mdioInfo.smi.smiAddress = portAttributes.phyData.smiXmsiInterface.address;
                                        break;
                                case PDL_PHY_TYPE_alaska_88E32x0_E:
                                case PDL_PHY_TYPE_alaska_88E33X0_E:
                                case PDL_PHY_TYPE_alaska_88E20X0_E:
                                case PDL_PHY_TYPE_alaska_88E2180_E:
                                        phy_entry.mdioType = MPD_MDIO_TYPE_XSMI_E;
                                        phy_entry.mdioInfo.xsmi.xsmiInterface = portAttributes.phyData.smiXmsiInterface.interfaceId;
                                        phy_entry.mdioInfo.xsmi.xsmiDev = portAttributes.phyData.smiXmsiInterface.dev;
                                        phy_entry.mdioInfo.xsmi.xsmiAddress = portAttributes.phyData.smiXmsiInterface.address;
                                        phy_entry.phyFw.fwType = MPD_FW_TYPE_SOLAR_FLARE_E;
                                        if (phy_representative_exists[portAttributes.phyData.phyNumber] == FALSE)
                                        {
                                                phy_entry.phyFw.isRepresentative = TRUE;
                                                phy_representative_exists[portAttributes.phyData.phyNumber] = TRUE;
                                        }
                                        break;

                                default:
                                        phy_entry.mdioType = MPD_MDIO_TYPE_OTHER_E;
                                        break;
                        }

                        phy_entry.phyType = phy_configuration.phyType;
                        phy_entry.transceiverType = portAttributes.transceiverType;
                        phy_entry.phySlice = portAttributes.phyData.phyPosition;
                        /*phy_entry.phyTxSerdesParams -> TBD update from PDL*/
                        /*phy_entry.shortReachSerdes -> TBD update from PDL */
                        /*phy_entry.abcdSwapRequired -> TBD update from PDL */
                        /*phy_entry.doSwapAbcd = TBD -> TBD update from PDL */
                }
#endif
                if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) == FALSE)
                {
                        return PDL_FAIL;
                }
                status = pdlPpDbPortAttributesGetNext(dev, port, &nextDev, &nextPort);
                rel_ifIndex++;
                dev = nextDev;
                port = nextPort;
        }

        return PDL_OK;

}

/*****************************************************************************
* FUNCTION NAME: utils_pdl_phy_init
*
* DESCRIPTION:         implementation of phy init for pdl
*
*
*****************************************************************************/
static PDL_STATUS utils_pdl_phy_init
(
)
{
        MPD_CALLBACKS_STC                   phy_callbacks;
        MPD_RESULT_ENT                      rc;
        PDL_STATUS                          pdlStatus;
        GT_U8                               i;

        /* MPD callbacks */
        phy_callbacks.sleep_PTR          = util_sleep;
        phy_callbacks.alloc_PTR          = (MPD_ALLOC_FUNC *)util_malloc;
        phy_callbacks.free_PTR           = free;
        phy_callbacks.debug_bind_PTR     = NULL;
        phy_callbacks.is_active_PTR      = NULL;
        phy_callbacks.debug_log_PTR      = NULL;
        phy_callbacks.txEnable_PTR       = util_phy_transceiver_enable;
        phy_callbacks.handle_failure_PTR = util_phy_handle_failure;
        phy_callbacks.getAdminStatus_PTR = NULL;
        phy_callbacks.getFwFiles_PTR     = util_phy_get_fw_files;

        /* initialize DBs & bind debug log ids */
        rc = mpdDriverInitDb(&phy_callbacks);
        if (rc != MPD_OK_E) {
                PRV_MPD_HANDLE_FAILURE_MAC( 0, MPD_ERROR_SEVERITY_FATAL_E, "mpdDriverInitDb failed");
        }
        /* update phy driver ports information into the hash */
        pdlStatus = utils_pdl_phy_init_ports_info();

        for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++){
                prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_FLASH_E;
        }
        mpdDriverInitHw();

        return pdlStatus;
}
#endif /*INCLUDE_MPD*/

/*****************************************************************************
* FUNCTION NAME: utils_pdl_init
*
* DESCRIPTION:         implementation of malloc for pdl
*
*
*****************************************************************************/
static PDL_STATUS utils_pdl_init
(
        IN char*                                  fileNamePtr,
        IN char*                                                xmlPrefix
)
{
        UINT_32                         fanControllerId;
    PDL_STATUS                                                pdlStatus, pdlStatus2;
    PDL_OS_CALLBACK_API_STC         callbacks;

    cpssOsMemSet(&callbacks,0,sizeof(callbacks));
    callbacks.printStringPtr        = cpssOsPrintf;
    callbacks.mallocPtr             = util_malloc;
    callbacks.freePtr               = free;
    callbacks.smiRegReadClbk        = util_smi_read;
    callbacks.smiRegWriteClbk       = util_smi_write;
    callbacks.xsmiRegReadClbk       = cpssXsmiRegisterRead;
    callbacks.xsmiRegWriteClbk      = cpssXsmiRegisterWrite;
    callbacks.ppCmRegReadClbk       = util_cm_register_read;
    callbacks.ppCmRegWriteClbk      = util_cm_register_write;
    callbacks.ppRegReadClbk         = util_register_read;
    callbacks.ppRegWriteClbk        = util_register_write;
    callbacks.arXmlUncompressClbk   = util_archive_uncompress;
    callbacks.vrXmlVerifyClbk                = util_xml_verification;

        pdlStatus = pdlInit (fileNamePtr, xmlPrefix, &callbacks, PDL_OS_INIT_TYPE_FULL_E);
    if (pdlStatus == PDL_OK)
    {
       pdlStatus = iDbgPdlInit(&callbacks,"tests");
       if (pdlStatus != PDL_OK)
           cpssOsPrintf("Can't initialize. iDbgPdlInit returned #[%d]", pdlStatus);

    }
    else
        cpssOsPrintf("Can't initialize. pdlInit returned #[%d]", pdlStatus);
#ifdef INCLUDE_MPD
        /* PHY init */
        pdlStatus = utils_pdl_phy_init();
        if (pdlStatus != PDL_OK)
        {
                cpssOsPrintf("Can't initialize PHY. utils_pdl_phy_init returned #[%d]", pdlStatus);
                return pdlStatus;
        }
#endif /*INCLUDE_MPD*/

    pdlInitDone();

    if (pdlStatus == PDL_OK)
    {
        /* initialize all fans */
        pdlStatus2 = pdlFanDbGetFirstController (&fanControllerId);
        while (pdlStatus2 == PDL_OK) {
            pdlStatus = pdlFanControllerHwInit(fanControllerId);
            if (pdlStatus != PDL_OK && pdlStatus != PDL_NOT_SUPPORTED) {
                cpssOsPrintf("%s - can't initialize fanController #[%d]", __FUNCTION__, fanControllerId);
                return pdlStatus;
            }
            pdlStatus2 = pdlFanDbGetNextController(fanControllerId, &fanControllerId);
        }
    }
        return pdlStatus;
}

/*************** WRAPPERS ************************/
/***********************************************************************************************************************************/
int wr_utils_pdl_init(lua_State *L) {
        int                       res;
        const char                                  * fileName;
        const char                                  * prefix;

        fileName = lua_tostring(L,1);
        prefix = lua_tostring(L,2);

        res = utils_pdl_init((char*)fileName, (char*)prefix);

        lua_pushnumber(L, res);
    return 1;
}
