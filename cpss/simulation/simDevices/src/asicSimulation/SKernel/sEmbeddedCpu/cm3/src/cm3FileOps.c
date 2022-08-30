/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/

/*******************************************************************************
* topLevelBoot.c
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
* DEPENDENCIES:
*
*******************************************************************************/

/* Includes */

#include <gtOs/gtGenTypes.h>
#include <cm3FileOps.h>
#include <cm3FileCommon.h>
#include <string.h>
#include <asicSimulation/SCIB/scib.h>





#define SUPER_IMAGE_NAME  "super_image_falcon.bin"
#define CM3_SIM_BOOTON_PROJECT_MAGIC "init" /*copy from BOOTON_PROJECT_MAGIC */

#include <stdio.h>
#include <stdlib.h>

/*Forward declaration*/
GT_STATUS simCm3ImageFileGet(BOOTONP_image_header_STC ** image_ptr_ptr,GT_BOOL directLoad);

/*
 0x00000000 -- 0x5fffffff: SRAM
 0x60000000 -- 0x7fffffff: MG0
 0x80000000 -- 0x9fffffff: MG0 moving window (with completion register)
 0xa0000000 -- 0xa00fffff: RUnit, internal access
 0xa0100000 -- 0xdfffffff: CM3 windows
*/
#define DDR_ADRESS_FOR_CM3_USAGE 0xB0100000 /*
                            The base (0x10000000) is added to 0xa0100000.
                            In a "real"  micro init defined by
                            {#define SPI_BASE_ADDR                       0xB0100000}
                            in a file  "fileCommon.h" */


/*
                            Adress of super image in DDR .
*/
#define SUPER_IMAGE_ADRESS_FOR_CM3_USAGE (DDR_ADRESS_FOR_CM3_USAGE+HEADER_FLASH_OFFSET_FOR_CM3_USAGE)


#define HOSTG_swap32_MAC(val)   ( MV_BYTE_SWAP_32BIT(val) )
#define HOSTG_swap16_MAC(val)   ( MV_BYTE_SWAP_16BIT(val) )



/* imports */


/* typedefs */

/* Locals */
GT_U8 num_of_files, file_iteration = 0;
uintptr_t  file_address;/* uintptr_t is 32-bits wide on a 32-bit machine, but it becomes 64-bits wide on a 64-bit machine.*/
BOOTONP_image_header_STC    *image_ptr = NULL;
BOOTONP_image_file_STC      *file_header_ptr;
static file_type             current_file_type = ignored;

/* ini values for double layered image */
GT_U32  imageOffset     = 0;
GT_U32  partitionSize   = 0;


/* Adress  of the image*/

static GT_BOOL logRedirect = GT_FALSE;

void snetChtPerformScibCm3DdrRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 *memPtr
);



/* Local routines */

/*******************************************************************************
* cm3GetNextFile
*
* DESCRIPTION:
*
* INPUT:
*      GT_BOOL restart - when true, scan starts from start of file list.
*
* OUTPUT:
*       file_params_STC *file_params_ptr
*
*
* RETURN:
*       GT_STATUS - MV_OK
*                   MV_ERROR
*******************************************************************************/
GT_BOOL cm3GetNextFile(file_params_STC *file_params_ptr, GT_BOOL restart)
{
    GT_U32 total_header_size;


    if (restart == GT_TRUE)
    {
        file_iteration = 0;
        /*Leave the option to load from FS directly if the adress in unitialized*/
        if(simCm3ImageFileGet(&image_ptr,GT_FALSE)!= GT_OK)
        {
            return GT_FALSE;
        }

        file_header_ptr = (BOOTONP_image_file_STC *)(image_ptr + 1);
        num_of_files = HOSTG_swap32_MAC(image_ptr->num_of_files);
        total_header_size = HOSTG_swap32_MAC(image_ptr->total_header_size);
        file_address = (uintptr_t)image_ptr + total_header_size;

    }

    if(file_iteration < num_of_files) {
        file_params_ptr->address = (uintptr_t)file_address;
        file_params_ptr->size = HOSTG_swap32_MAC(file_header_ptr->file_size);
        file_params_ptr->type = HOSTG_swap32_MAC(file_header_ptr->type);
        file_params_ptr->name = (char *)file_header_ptr->file_name;
        file_params_ptr->exec_cpy_offset = HOSTG_swap32_MAC(file_header_ptr->exec_cpy_offset);
        file_params_ptr->exec_offset = HOSTG_swap32_MAC(file_header_ptr->exec_offset);
        file_params_ptr->image_offset = HOSTG_swap32_MAC(file_header_ptr->image_offset);
        file_params_ptr->useBmp = HOSTG_swap32_MAC(file_header_ptr->bmp);

        file_header_ptr++;
        file_address = file_address + file_params_ptr->size;
        file_iteration++;
        current_file_type = file_params_ptr->type;

        return GT_TRUE;
    }
    current_file_type = ignored;
    return GT_FALSE;
}

/*******************************************************************************
* getCurrentFileType -
*
* DESCRIPTION:
*       returns current processed file type by getNextFile
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       In case a file is processed  - the file type
*       Otherwise - ignored type
*******************************************************************************/
file_type getCurrentFileType(void)
{
    return current_file_type;
}
/*******************************************************************************
* simCm3imageFileGetFromDma
*
* DESCRIPTION:Load super image file into CM3 RAM from file system directly(No PEX mapping )
*
* INPUT:
*
*
* OUTPUT:
*       file_params_STC *file_params_ptr
*
*
* RETURN:
*       GT_STATUS - MV_OK
*                   MV_ERROR
*******************************************************************************/
GT_STATUS simCm3ImageFileGetFromFs(BOOTONP_image_header_STC ** image_ptr_ptr)
{
   FILE * fp = NULL;
   long fsize;
   void * buffer = NULL;
   size_t sizeRead;



   fp = fopen(SUPER_IMAGE_NAME,"rb");
   if(fp==NULL)
   {
       SIM_CM3_LOG("Error opening %s file\n",SUPER_IMAGE_NAME);
       return GT_FAIL;
   }

   fseek(fp, 0, SEEK_END);
   fsize = ftell(fp);
   fseek(fp, 0, SEEK_SET);  /* rewind(f) */

   buffer = malloc(fsize + 1);
   if(buffer==NULL)
   {
       SIM_CM3_LOG("Error allocating  %d bytes in order to emulate  flash for %s \n",fsize,SUPER_IMAGE_NAME);
       return GT_FAIL;
   }

   memset(buffer,0,fsize);

   SIM_CM3_LOG("Allocating  %d bytes in order to emulate  flash for %s \n",fsize,SUPER_IMAGE_NAME);

   sizeRead = fread(buffer,fsize,1,fp);
   if (sizeRead != 1)
   {
     SIM_CM3_LOG ("Reading error");
     return GT_FAIL;
   }
   *image_ptr_ptr = (BOOTONP_image_header_STC *)(buffer);

   fclose(fp);

   return GT_OK;

}

GT_U32   supperImageSize;

/*******************************************************************************
* simCm3imageFileGetFromDdr
*
* DESCRIPTION:Load super image file into CM3 RAM from defined PEX adress
*
* INPUT:
*
*
* OUTPUT:
*       file_params_STC *file_params_ptr
*
*
* RETURN:
*       GT_STATUS - MV_OK
*                   MV_ERROR
*******************************************************************************/

GT_STATUS simCm3imageFileGetFromDdr(BOOTONP_image_header_STC ** image_ptr_ptr)
{
   GT_U32 headerSize,sizeToAllocate,wordsToRead,i;
   BOOTONP_image_header_STC tmp;
   unsigned char buffer[HOSTG_IMAGE_MAGIC_LENGTH+1];

   headerSize = sizeof(BOOTONP_image_header_STC);

   SIM_CM3_LOG("Loading super image from PEX adress 0x%x\n",SUPER_IMAGE_ADRESS_FOR_CM3_USAGE);

   /*First lets get the size of image */

   snetChtPerformScibCm3DdrRead(SIM_CM3_DEVICE_ID,
    SUPER_IMAGE_ADRESS_FOR_CM3_USAGE,(headerSize>>2)+headerSize%2,(GT_U32 *)&tmp);

   for(i=0;i<HOSTG_IMAGE_MAGIC_LENGTH;i++)
   {
        buffer[i]= tmp.image_magic[i];
   }
   buffer[HOSTG_IMAGE_MAGIC_LENGTH]='\0';

   if(GT_FALSE == microInitCm3ValidateSuperImage(buffer,GT_FALSE))
   {
        return GT_FAIL;
   }

   sizeToAllocate = HOSTG_swap32_MAC(tmp.image_size);

   SIM_CM3_LOG("File size is %d \n",sizeToAllocate);

   /*Second lets allocate buffer*/

   *image_ptr_ptr = (BOOTONP_image_header_STC *)malloc(sizeToAllocate + 1);

   if(*image_ptr_ptr==NULL)
   {
       SIM_CM3_LOG("Error allocating  %d bytes \n",sizeToAllocate + 1);
       return GT_FAIL;
   }

   supperImageSize = sizeToAllocate;/*value in bytes*/

   wordsToRead = (sizeToAllocate>>2)+headerSize%2;

   snetChtPerformScibCm3DdrRead(SIM_CM3_DEVICE_ID,
    SUPER_IMAGE_ADRESS_FOR_CM3_USAGE,wordsToRead,(GT_U32 *)(*image_ptr_ptr));


   return GT_OK;

}
/*******************************************************************************
* simCm3ImageFileGet
*
* DESCRIPTION: Load super image file into CM3 RAM
*
* INPUT:
*      GT_BOOL directLoad - when true file is loaded directly from file system,PEX is bypassed.
*                                               when false file is loaded from PEX adress . Note that in this case PEX adress
*                                               should be configured first.
*
* OUTPUT:
*       file_params_STC *file_params_ptr
*
*
* RETURN:
*       GT_STATUS - GT_OK
*                   GT_FAIL
*******************************************************************************/
GT_STATUS simCm3ImageFileGet(BOOTONP_image_header_STC ** image_ptr_ptr,GT_BOOL directLoad)
{
    SIM_CM3_LOG("Loading file from  %s \n",directLoad?"file system":"DDR throught PEX.");
    if(GT_TRUE == directLoad)
    {
        return simCm3ImageFileGetFromFs(image_ptr_ptr);
    }

    return simCm3imageFileGetFromDdr(image_ptr_ptr);
}

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
)
{
    logRedirect = redirectLogToStdout;
    return GT_OK;
}
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
)
{
    return logRedirect ;
}


/*******************************************************************************
* microInitCm3ValidateSuperImage (copy from BOOTON_validate_Super_Image)
*
* DESCRIPTION:
*               This routine validate Super_Image.bin (MI, FW_SDK, etc) file.
*               Check size, header & image crc.
*
* INPUT:
*
*       hideOutput  -    MV_TRUE to hide output thru serial,
*                             to avoid collisions with xmodem protocol
*
* RETURN:
*       MV_TRUE - The Super_Image.bin is OK.
*
*******************************************************************************/
GT_BOOL microInitCm3ValidateSuperImage(unsigned char * magic,GT_BOOL hideOutput)
{


    /* Validate correct magic code */
    if (strcmp((const char *)magic, CM3_SIM_BOOTON_PROJECT_MAGIC) != 0){
        if (hideOutput == GT_FALSE) {

            SIM_CM3_LOG("\nWrong magic! %s\n\r",magic);
        }
        return GT_FALSE;
    }

    return GT_TRUE;
} /* BOOTON_validate_Super_Image */

/*******************************************************************************
* getImageVersion -
*
* DESCRIPTION:
*       returns pointer to image version in BOOTONP_image_file_STC structure
*
* INPUT:
*      None.
*
* OUTPUT:
*      imagePartition - the partition the image resides in
*
* RETURN:
*       In case of success - pointer to image version in BOOTONP_image_file_STC structure
*       Otherwise - NULL
*******************************************************************************/
GT_U8* getImageVersion(GT_U8 *imagePartition)
{
    BOOTONP_image_header_STC *image = NULL;

    image = image_ptr /*(BOOTONP_image_header_STC *)(SPI_BASE_ADDR + (HEADER_FLASH_OFFSET + imageOffset))*/;
    if (image) {
        if (imagePartition)
            *imagePartition = imageOffset / partitionSize;
        return image->image_version;
    }
    return NULL;
}


