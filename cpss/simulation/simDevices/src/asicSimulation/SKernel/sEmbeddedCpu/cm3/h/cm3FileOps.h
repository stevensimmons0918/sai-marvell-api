
#ifndef __cm3fileOps
#define __cm3fileOps

#include <cm3FileCommon.h>
/*
HEADER_FLASH_OFFSET  in a file  "fileCommon.h"
*/
#define HEADER_FLASH_OFFSET_FOR_CM3_USAGE                 0x40000 /* 256KB */

/* file_params_STC:

        address             - holds address to start of file
    size                - size of file
        type                - type of file - enum: { CM3_FW, EPROM, script, AVAGO_FW_sbus, AVAGO_FW_spico, micro_init, ignored }
        name                - file name
        useBmp              - file usage bitmap
*/

typedef struct file_params_STCT
{
    uintptr_t address;
    GT_U32 size;
    file_type type;
    char *name;
    GT_U32 exec_cpy_offset;
    GT_U32 exec_offset;
    GT_U32 image_offset;
    GT_U32 useBmp;


}file_params_STC;


typedef struct {

    GT_U32      magic;              /*  0-3  */
    GT_U8       version;            /*   4   */
    GT_U8       hdrSize;            /*   5   */
    GT_U8       flags;              /*   6   */
    GT_U8       spiAddrWidth;       /*   7   */
    GT_U32      imageOffset;        /*  8-11 */
    GT_U32      imageSize;          /* 12-15 */
    GT_U32      imageChksum;        /* 16-19 */
    GT_U32      rsvd1;              /* 20-23 */
    GT_U32      rsvd2;              /* 24-27 */
    GT_U16      rsvd3;              /* 28-29 */
    GT_U8       ext;                /*   30  */
    GT_U8       hdrChksum;          /*   31  */

} bootonHeader;


/*******************************************************************************
* cm3GetNextFile -
*
* DESCRIPTION:
*
* INPUT:
*      MV_BOOL restart - when true, scan starts from start of file list.
*
* OUTPUT:
*       file_params_STC *file_params_ptr
*
*
* RETURN:
*       MV_STATUS - MV_OK
*                   MV_ERROR
*******************************************************************************/
GT_STATUS cm3GetNextFile(file_params_STC *file_params_ptr, GT_BOOL restart);

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
file_type getCurrentFileType(void);
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
GT_BOOL microInitCm3ValidateSuperImage(unsigned char * magic,GT_BOOL hideOutput);

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
GT_U8* getImageVersion(GT_U8 *imagePartition);



#endif  /* __miniCm3fileOps */
