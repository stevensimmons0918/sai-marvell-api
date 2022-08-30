/**
********************************************************************************
* @file cmdFS_RAM.c
*
* @brief RAM cmdFS implementation
*
* @version   19
********************************************************************************
*/
/****************************************************
* cmdFS_RAM.c
*
* DESCRIPTION:
*       RAM cmdFS implementation
*
* DEPENDENCIES:
*
* COMMENTS:
*       This FS has no protection for simultaneous write:
*       there should be only one writer at time and no readers
*       when writer active
*
* FILE REVISION NUMBER:
*       $Revision: 19 $
*******************************************************************************/

/***** Include files ***************************************************/
#include <cmdShell/FS/cmdFS_RAM.h>

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cmdShell/FS/cmdFS.h>
#if defined(ENV_CPSS)
#include <extUtils/FS/genFS.h>
#else
#include <cmdShell/FS/cmdFS.h>
#endif

#ifndef __KERNEL__
#define ZLIB_SUPPORT
#endif
#ifdef ZLIB_SUPPORT
#include "zlib/zlib.h"
#define ZLIB_COMPR_BUFFER_SIZE 0x4000
#endif

/***** Defines  ********************************************************/
#define CMD_FS_MAX_OPEN_FILES   10
#define CMD_FS_O_DIRECTORY      0x40000000

typedef struct CMD_FS_FDESCR_STC {
    int                 flags;      /* open flags, 0 means not opened */
    CMD_FS_INODE_STC    *inode;
    int                 pos;        /* current position */
    CMD_FS_CHUNK_STC    *chunk;     /* current chunk */
    int                 chunkpos;   /* position in current chunk */
#ifdef ZLIB_SUPPORT
    z_stream            *strm;
#endif
} CMD_FS_FDESCR_STC;


/***** Private Data ****************************************************/
static CMD_FS_INODE_STC *cmdFSdir = NULL;
static CMD_FS_FDESCR_STC    fdescr[CMD_FS_MAX_OPEN_FILES];
static const char *last_error = "";


void CMDFS_RAM_INIT_EMBEDDED(void);
void CMDFS_RAM_INIT_EMBEDDED_XML(void);
void CMDFS_RAM_INIT_EMBEDDED_EXAMPLES(void);
#ifdef CMDFS_RAM_EMBEDDED_DATA_EXT_XML_EXT
void CMDFS_RAM_INIT_EMBEDDED_XML_C005(void);
#endif

/***** Private Functions ***********************************************/

/*******************************************************************************
* CMD_FS_CHECK_FD
*
* DESCRIPTION:
*       Check that fd is valid opened file descriptor
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       -1  - on error
*
* COMMENTS:
*
*******************************************************************************/
#define FD                  fdescr[fd]
#define CMD_FS_CHECK_FD() \
    if (fd < 0 || fd >= CMD_FS_MAX_OPEN_FILES) \
    { \
        last_error = "Wrong file descriptor"; \
        return -1; \
    } \
    if (FD.flags == 0) \
    { \
        last_error = "Bad file descriptor"; \
        return -1; \
    }

/*******************************************************************************
* cmdFS_RAM_lookupFile
*
* DESCRIPTION:
*       Scan directory linked list for given file
*
* INPUTS:
*       name    - filename
*
* OUTPUTS:
*       None
*
* RETURNS:
*       inode pointer
*       - NULL if error
*
* COMMENTS:
*
*******************************************************************************/
static CMD_FS_INODE_STC* cmdFS_RAM_lookupFile(const char *name)
{
    CMD_FS_INODE_STC  *inode;

    for (inode = cmdFSdir; inode; inode = inode->next)
    {
        if (!CMD_FS_STRCMP(inode->name, name))
        {
            return inode;
        }
    }
    return NULL;
}

/*******************************************************************************
* cmdFS_RAM_newFileEntry
*
* DESCRIPTION:
*       Allocate inode for new file and link it into directory
*
* INPUTS:
*       name    - filename
*
* OUTPUTS:
*       None
*
* RETURNS:
*       inode pointer
*       - NULL if error
*
* COMMENTS:
*
*******************************************************************************/
CMD_FS_INODE_STC* cmdFS_RAM_newFileEntry(const char* name)
{
    CMD_FS_INODE_STC *inode;
    inode = cmdFSdir;
    if (inode)
    {
        while(inode->next)
            inode = inode->next;
        inode->next = (CMD_FS_INODE_STC*) CMD_FS_MALLOC(sizeof(*inode));
        inode = inode->next;
    } else {
        inode = cmdFSdir = (CMD_FS_INODE_STC*) CMD_FS_MALLOC(sizeof(*inode));
    }
    if (inode == NULL)
        return inode;
    CMD_FS_BZERO(inode, sizeof(*inode));
    inode->name = CMD_FS_STRDUP(name);
    inode->ctime = CMD_FS_TIME();
    inode->mtime = inode->ctime;
    inode->nrefs = 1;
    return inode;
}

/*******************************************************************************
* cmdFS_RAM_AddEmbeddedFileEntry
*
* DESCRIPTION:
*       Allocate inode for new Embedded file and link it into directory
*
* INPUTS:
*       name    - filename
*
* OUTPUTS:
*       None
*
* RETURNS:
*       inode pointer
*       - NULL if error
*
* COMMENTS:
*
*******************************************************************************/
CMD_FS_INODE_STC* cmdFS_RAM_AddEmbeddedFileEntry(const char* name, size_t size, unsigned mtime)
{
    CMD_FS_INODE_STC *inode;
    inode = cmdFS_RAM_newFileEntry(name);
    inode->flags |= CMD_FS_FLAGS_EMBEDDED;
    inode->nrefs = 1;
    inode->size = size;
    inode->mtime = mtime;
    return inode;
}

/**
* @internal cmdFS_RAM_truncate function
* @endinternal
*
* @brief   Truncate file - delete all data chunks
*/
static void cmdFS_RAM_truncate(CMD_FS_INODE_STC *inode)
{
    if (inode->flags & CMD_FS_FLAGS_EMBEDDED)
    {
        inode->flags &= ~CMD_FS_FLAGS_EMBEDDED;
#ifdef ZLIB_SUPPORT
        inode->flags &= ~CMD_FS_FLAGS_COMPRESSED;
#endif
    }
    else
    {
        CMD_FS_CHUNK_STC *data;

        data = inode->data;
        while (data)
        {
            CMD_FS_CHUNK_STC *next = data->next;
            CMD_FS_FREE(data);
            data = next;
        }
    }
    inode->data = NULL;
    inode->size = 0;
}

/*******************************************************************************
* cmdFS_RAM_allocChunk
*
* DESCRIPTION:
*       Allocate chunk for file data
*
* INPUTS:
*       size    - chunk size
*
* OUTPUTS:
*       None
*
* RETURNS:
*       chunk pointer
*
* COMMENTS:
*
*******************************************************************************/
static CMD_FS_CHUNK_STC* cmdFS_RAM_allocChunk(int size)
{
    CMD_FS_CHUNK_STC *chunk;
    if (size == 0)
        return NULL;
    chunk = (CMD_FS_CHUNK_STC *)CMD_FS_MALLOC(sizeof(CMD_FS_CHUNK_STC)+size);
    if (chunk == NULL)
        return chunk;
    chunk->chunksize = size;
    chunk->next = NULL;
    return chunk;
}

/**
* @internal cmdFS_RAM_dereference_inode function
* @endinternal
*
* @brief   Dereference inode: redude nrefs counter and destroy inode if
*         no mode references left
*/
static void cmdFS_RAM_dereference_inode(CMD_FS_INODE_STC *inode)
{
    inode->nrefs--;

    if (inode->nrefs > 0)
    {
        /* at least one referece left */
        return;
    }
    /* no more references */
    cmdFS_RAM_truncate(inode);
    CMD_FS_FREE(inode->name);
    CMD_FS_FREE(inode);
}

/***** Public Functions ************************************************/

/**
* @internal cmdFSinit function
* @endinternal
*
* @brief   Initialize cmdFS, initialize built-in files
*
* @retval 0                        - on success
*/
int cmdFSinit(void)
{
    static int already_initialized = 0;
    if (already_initialized)
    {
        return 0;
    }
    /* init built-in files */
    CMDFS_RAM_INIT_EMBEDDED();
    CMDFS_RAM_INIT_EMBEDDED_XML();
    CMDFS_RAM_INIT_EMBEDDED_EXAMPLES();
#ifdef CMDFS_RAM_EMBEDDED_DATA_EXT_XML_EXT
    CMDFS_RAM_INIT_EMBEDDED_XML_C005();
#endif

    {
        /* mark all files */
        CMD_FS_INODE_STC  *inode;

        for (inode = cmdFSdir; inode; inode = inode->next)
        {
            inode->flags |= CMD_FS_FLAGS_EMBEDDED;
            inode->nrefs = 1;
        }
    }

    CMD_FS_BZERO(fdescr, sizeof(fdescr));
    already_initialized = 1;
    return 0;
}

/*******************************************************************************
* cmdFSlastError
*
* DESCRIPTION:
*       Return string with last error description
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0   - on success
*
* COMMENTS:
*
*******************************************************************************/
static const char* cmdFSlastError(void)
{
    return last_error;
}

#ifdef ZLIB_SUPPORT
static voidpf cmdFS_zcalloc (voidpf opaque, uInt items, uInt size)
{
    if (opaque) items += size - size; /* make compiler happy */
    return (voidpf)CMD_FS_MALLOC(items * size);
}

static void cmdFS_zcfree (voidpf opaque, voidpf ptr)
{
    CMD_FS_FREE(ptr);
    if (opaque) return; /* make compiler happy */
}
#endif
/**
* @internal cmdFSopen function
* @endinternal
*
* @brief   Open and possible create a new file
*
* @note read POSIX open() as reference
*
*/
static int cmdFSopen(const char* name, int flags)
{
    int fd;
    CMD_FS_INODE_STC  *inode;

    if (flags == 0 || (flags & CMD_FS_O_DIRECTORY))
    {
        last_error = "open(): wrong flags";
        return CPSS_OS_FILE_INVALID;
    }
#ifdef ZLIB_SUPPORT
    if ((flags & GEN_FS_O_WRONLY) && (flags & GEN_FS_O_RDONLY)
            && (flags & GEN_FS_O_COMPRESS))
    {
        last_error = "open(): w+c is not supported";
        return CPSS_OS_FILE_INVALID;
    }
#endif
    /* look for new fd */
    for (fd = 0; fd < CMD_FS_MAX_OPEN_FILES; fd++)
    {
        if (FD.flags == 0)
        {
            break;
        }
    }
    if (fd >= CMD_FS_MAX_OPEN_FILES)
    {
        last_error = "No enough descriptors";
        return -1;
    }
    /* unused file descriptor found. lock it first */
    FD.flags = -1; /* lock */

    inode = cmdFS_RAM_lookupFile(name);
    if (inode == NULL)
    {
        /* file not yet exists */
        if ((flags & GEN_FS_O_CREAT) == 0)
        {
            FD.flags = 0; /* unlock */
            last_error = "No such file";
            return -1;
        }
        if ((flags & GEN_FS_O_WRONLY) == 0)
        {
            FD.flags = 0; /* unlock */
            last_error = "Wrong open flags (O_CREAT w/o write)";
            return -1;
        }
        inode = cmdFS_RAM_newFileEntry(name);
        if (inode == NULL)
        {
            FD.flags = 0; /* unlock */
            last_error = "Failed to create new file (can't alloc)";
            return -1;
        }
    }
    else
    {
        /* file exists */
        if ((flags & GEN_FS_O_CREAT) && (flags & GEN_FS_O_EXCL))
        {
            FD.flags = 0; /* unlock */
            last_error = "File already exists";
            return -1;

        }
        if (flags & GEN_FS_O_WRONLY)
        {
            if ((inode->flags & CMD_FS_FLAGS_EMBEDDED) && !(flags & GEN_FS_O_TRUNC))
            {
                /* write to embedded file without truncation */
                FD.flags = 0; /* unlock */
                last_error = "permission denied";
                return -1;
            }
        }
    }
    /* truncate file if O_TRUNC and writable */
    if ((flags & GEN_FS_O_TRUNC) && (flags & GEN_FS_O_WRONLY))
    {
        /* O_TRUNK and write allowed */
        cmdFS_RAM_truncate(inode);

    }
#ifdef ZLIB_SUPPORT
    FD.strm = NULL;
    if ((flags & GEN_FS_O_RDONLY) && (inode->flags & CMD_FS_FLAGS_COMPRESSED))
    {
        FD.strm = CMD_FS_MALLOC(sizeof(z_stream));
        if (FD.strm == NULL)
        {
            FD.flags = 0; /* unlock */
            last_error = "no enough memory to uncompress";
            return -1;
        }
        CMD_FS_BZERO(FD.strm, sizeof(z_stream));
        FD.strm->next_in = (Bytef*)inode->data->data;
        FD.strm->avail_in = (uInt)inode->data->chunksize;
        FD.strm->zalloc = cmdFS_zcalloc;
        FD.strm->zfree = cmdFS_zcfree;
        if (inflateInit(FD.strm) != Z_OK)
        {
            CMD_FS_FREE(FD.strm);
            FD.flags = 0; /* unlock */
            last_error = "failed to inflateInit()";
            return -1;
        }
    }
    while ((flags & GEN_FS_O_WRONLY) && (flags & GEN_FS_O_COMPRESS) && inode->data == NULL)
    {
        /* new/empty file, enable compression */
        FD.strm = CMD_FS_MALLOC(sizeof(z_stream));
        if (FD.strm == NULL)
        {
            FD.flags = 0; /* unlock */
            last_error = "no enough memory to compress";
            return -1;
        }
        CMD_FS_BZERO(FD.strm, sizeof(z_stream));
        FD.strm->zalloc = cmdFS_zcalloc;
        FD.strm->zfree = cmdFS_zcfree;
        FD.strm->opaque = (voidpf)CMD_FS_MALLOC(ZLIB_COMPR_BUFFER_SIZE);
        if (FD.strm->opaque == NULL)
        {
            CMD_FS_FREE(FD.strm);
            flags &= ~GEN_FS_O_COMPRESS;
            FD.strm = NULL;
            break;
        }
        if (deflateInit(FD.strm,Z_DEFAULT_COMPRESSION) != Z_OK)
        {
            /* don't compress */
            CMD_FS_FREE(FD.strm->opaque);
            CMD_FS_FREE(FD.strm);
            flags &= ~GEN_FS_O_COMPRESS;
            FD.strm = NULL;
            break;
        }
        FD.strm->next_out = (Bytef*)FD.strm->opaque;
        FD.strm->avail_out = ZLIB_COMPR_BUFFER_SIZE;
        inode->flags |= CMD_FS_FLAGS_COMPRESSED;
        break;
    }
#endif

    /* initialize file descriptor */
    FD.flags = flags;
    FD.inode = inode;
    inode->nrefs++;
    FD.pos = 0;
    FD.chunk = inode->data;
    FD.chunkpos = 0;

    return fd;
}

#ifdef ZLIB_SUPPORT
/**
* @internal cmdFS_z_flush_out function
* @endinternal
*
* @brief   Flush zlib output buffer to data chain
*/
static int cmdFS_z_flush_out(int fd)
{
    CMD_FS_CHUNK_STC    *chunk;
    int                 flushsize;
    flushsize = ZLIB_COMPR_BUFFER_SIZE - FD.strm->avail_out;

    if (flushsize == 0)
    {
        return 0;
    }

    chunk = cmdFS_RAM_allocChunk(flushsize);
    if (chunk == NULL)
    {
        last_error = "cmdFSwrite_z(): failed to alloc";
        return -1;
    }
    CMD_FS_BCOPY(FD.strm->opaque, chunk->data, flushsize);
    if (FD.inode->data == NULL)
    {
        /* file is empty, create first chunk */
        FD.inode->data = chunk;
        FD.chunk = FD.inode->data;
        FD.chunkpos = (int)chunk->chunksize;
    }
    else
    {
        FD.chunk->next = chunk;
        FD.chunk = FD.chunk->next;
        FD.chunkpos = (int)chunk->chunksize;
    }
    FD.strm->next_out = (Bytef*)FD.strm->opaque;
    FD.strm->avail_out = ZLIB_COMPR_BUFFER_SIZE;
    return 0;
}
#endif

/**
* @internal cmdFSclose function
* @endinternal
*
* @brief   Close a file descriptor
*
* @note read POSIX close() as reference
*
*/
static int cmdFSclose(int fd)
{
    CMD_FS_CHECK_FD();

#ifdef ZLIB_SUPPORT
    if (FD.strm != NULL)
    {
        if (FD.flags & GEN_FS_O_RDONLY)
        {
            inflateEnd(FD.strm);
        }
        else
        {
            /* flush */
            int res = Z_OK;

            while (res == Z_OK)
            {
                if (FD.strm->avail_out == 0)
                {
                    if (cmdFS_z_flush_out(fd) < 0)
                        break;
                }
                res = deflate(FD.strm, Z_FINISH);
            }
            cmdFS_z_flush_out(fd);

            deflateEnd(FD.strm);
            CMD_FS_FREE(FD.strm->opaque);
        }
        CMD_FS_FREE(FD.strm);
        FD.strm = NULL;
    }
#endif
    if ((FD.flags & CMD_FS_O_DIRECTORY) == 0)
    {
        /* file */
        cmdFS_RAM_dereference_inode(FD.inode);
    }
    FD.flags = 0;
    return 0;
}

#ifdef ZLIB_SUPPORT
/**
* @internal cmdFSread_z function
* @endinternal
*
* @brief   read from a file descriptor (decompress transparently)
*/
static int cmdFSread_z(int fd, void *buf, int count)
{
    FD.strm->next_out = buf;
    FD.strm->avail_out = count;
    while (FD.strm->avail_out > 0)
    {
        if (FD.strm->avail_in == 0)
        {
            /* current position is end of chunk */
            if (FD.chunk->next == NULL)
            {
                /* no more chunks, eof */
                break;
            }
            FD.chunk = FD.chunk->next;
            FD.strm->next_in = (Bytef*)FD.chunk->data;
            FD.strm->avail_in = (uInt)FD.chunk->chunksize;
        }
        inflate(FD.strm,Z_SYNC_FLUSH);
    }
    return count - FD.strm->avail_out;
}
#endif /* ZLIB_SUPPORT */

/**
* @internal cmdFSread function
* @endinternal
*
* @brief   read from a file descriptor
*
* @note read POSIX read() as reference
*
*/
static int cmdFSread(int fd, void *buf, int count)
{
    char    *optr = (char*)buf;
    int     nread = 0;

    CMD_FS_CHECK_FD();

    if (count < 0)
    {
        last_error = "Bad parameter: count";
        return -1;
    }
    if (buf == NULL)
    {
        last_error = "Bad parameter: buf";
        return -1;
    }
    if ((FD.flags & GEN_FS_O_RDONLY) == 0)
    {
        last_error = "File is not opened for reading";
        return -1;
    }
#ifdef ZLIB_SUPPORT
    if (FD.strm)
    {
        return cmdFSread_z(fd, buf, count);
    }
#endif
    if (FD.chunk == NULL)
    {
        /* eof */
        return 0;
    }
    while (count > 0)
    {
        int n;
        /* read from current chunk starting chunkpos till end of chunk
         * but no more than count bytes
         */
        n = (int)(FD.chunk->chunksize - FD.chunkpos);
        if (n > count)
            n = count;
        if (n)
        {
            CMD_FS_BCOPY(FD.chunk->data+FD.chunkpos, optr, n);
            FD.chunkpos += n;
            FD.pos += n;
            optr += n;
            nread += n;
            count -= n;
        }

        if (FD.chunkpos == (int)FD.chunk->chunksize)
        {
            /* current position is end of chunk */
            if (FD.chunk->next == NULL)
            {
                /* no more chunks, eof */
                return nread;
            }
            FD.chunk = FD.chunk->next;
            FD.chunkpos = 0;
        }
    }
    return nread;
}

#ifdef ZLIB_SUPPORT
/**
* @internal cmdFSwrite_z function
* @endinternal
*
* @brief   Write to a file descriptor (with compression)
*/
static int cmdFSwrite_z(int fd, const void *buf, int count)
{
    if (count == 0)
        return 0;

    FD.strm->next_in = CMD_FS_CONST_CAST(buf,Bytef*);
    FD.strm->avail_in = count;

    while (FD.strm->avail_in)
    {
        int res = deflate(FD.strm, Z_NO_FLUSH);
        if (res != Z_OK)
        {
            /*TODO:handle err*/
            last_error = "cmdFSwrite_z() deflate error";
            return -1;
        }
        if (FD.strm->avail_out == 0)
        {
            res = cmdFS_z_flush_out(fd);
            if (res < 0)
            {
                return res;
            }
        }
    }

    FD.pos += count;
    FD.inode->size += count;
    FD.inode->mtime = CMD_FS_TIME();
    return count;
}
#endif /* ZLIB_SUPPORT */

/**
* @internal cmdFSwrite function
* @endinternal
*
* @brief   Write to a file descriptor
*
* @note read POSIX write() as reference
*
*/
static int cmdFSwrite(int fd, const void *buf, int count)
{
    const char  *iptr = (const char*)buf;
    int         nwritten = 0;

    CMD_FS_CHECK_FD();

    if (count < 0)
    {
        last_error = "Bad parameter: count";
        return -1;
    }
    if (buf == NULL)
    {
        last_error = "Bad parameter: buf";
        return -1;
    }
    if ((FD.flags & GEN_FS_O_WRONLY) == 0)
    {
        last_error = "File is not opened for writing";
        return -1;
    }
#ifdef ZLIB_SUPPORT
    if (FD.strm)
    {
        return cmdFSwrite_z(fd, buf, count);
    }
#endif

    if (FD.inode->data == NULL)
    {
        /* file is empty, create first chunk */
        FD.inode->data = cmdFS_RAM_allocChunk(count);
        if (FD.inode->data == NULL)
        {
            last_error = "cmdFSwrite(): failed to alloc";
            return -1;
        }
        FD.chunk = FD.inode->data;
        CMD_FS_BCOPY(buf, FD.chunk->data, count);
        FD.chunkpos = count;
        FD.pos = count;
        FD.inode->mtime = CMD_FS_TIME();
        return count;
    }

    if (FD.flags & GEN_FS_O_APPEND)
    {
        /* seek to the end */
        while (FD.chunk->next != 0)
        {
            FD.pos += (int)(FD.chunk->chunksize - FD.chunkpos);
            FD.chunk = FD.chunk->next;
            FD.chunkpos = 0;
        }
        FD.pos += (int)(FD.chunk->chunksize - FD.chunkpos);
        FD.chunkpos = (int)(FD.chunk->chunksize);
    }

    while (count > 0)
    {
        int n;
        if (FD.chunkpos == (int)FD.chunk->chunksize)
        {
            /* current position is end of chunk */
            if (FD.chunk->next == NULL)
            {
                /* eof, allocate chunk for rest of data */
                FD.chunk->next = cmdFS_RAM_allocChunk(count);
                if (FD.chunk->next == NULL)
                {
                    last_error = "cmdFSwrite(): failed to alloc";
                    return -1;
                }
                FD.chunk = FD.chunk->next;
                CMD_FS_BCOPY(iptr, FD.chunk->data, count);
                FD.chunkpos = count;
                FD.pos += count;
                nwritten += count;
                FD.inode->mtime = CMD_FS_TIME();
                break;
            }
            /* get next chunk */
            FD.chunk = FD.chunk->next;
            FD.chunkpos = 0;
        }
        /* write to current chunk starting chunkpos till end of chunk
         * but no more than count bytes
         */
        n = (int)(FD.chunk->chunksize - FD.chunkpos);
        if (n > count)
            n = count;
        if (n)
        {
            CMD_FS_BCOPY(iptr, FD.chunk->data+FD.chunkpos, n);
            FD.inode->mtime = CMD_FS_TIME();
            FD.chunkpos += n;
            FD.pos += n;
            iptr += n;
            nwritten += n;
            count -= n;
        }
    }
    return nwritten;
}


/**
* @internal cmdFSlseek function
* @endinternal
*
* @brief   reposition read/write file offset
*
* @note read POSIX lseek() as reference
*
*/
static int cmdFSlseek(int fd, int offset GT_UNUSED, int whence GT_UNUSED)
{
    CMD_FS_CHECK_FD();
    last_error = "Not implemented";
    return -1;
}






/**
* @internal cmdFSstat function
* @endinternal
*
* @brief   get file status
*
* @note read POSIX stat() as reference
*
*/
static int cmdFSstat(const char* name, GEN_FS_STAT_STC *buf)
{
    CMD_FS_INODE_STC  *inode;
    CMD_FS_CHUNK_STC *chunk;

    inode = cmdFS_RAM_lookupFile(name);
    if (!inode)
    {
        last_error = "File not found";
        return -1;
    }
    CMD_FS_BZERO(buf, sizeof(*buf));
    buf->st_mode = GEN_FS_S_IFREG; /* regular file */
    buf->st_ctime = inode->ctime;
    buf->st_mtime = inode->mtime;

#ifdef ZLIB_SUPPORT
    if (inode->flags & CMD_FS_FLAGS_COMPRESSED)
    {
        buf->st_size = (int)inode->size;
    }
    else
#endif
    for (chunk = inode->data; chunk; chunk = chunk->next)
    {
        buf->st_size += (int)(chunk->chunksize);
    }
    return 0;
}


/**
* @internal cmdFSunlink function
* @endinternal
*
* @brief   delete a name and possibly the file it refers to
*
* @retval 0                        - on success
*
* @note read POSIX unlink() as reference
*
*/
static int cmdFSunlink(const char* name)
{
    CMD_FS_INODE_STC *inode;

    inode = cmdFS_RAM_lookupFile(name);
    if (!inode)
    {
        last_error = "File not found";
        return -1;
    }

    /* remove from dir inode chain */
    if (cmdFSdir == inode)
    {
        cmdFSdir = inode->next;
    } else {
        CMD_FS_INODE_STC *d = cmdFSdir;
        while (d->next != inode)
            d = d->next;
        d->next = inode->next;
    }
    cmdFS_RAM_dereference_inode(inode);
    return 0;
}

/**
* @internal cmdFSopendir function
* @endinternal
*
* @brief   open a directory
*/
static int cmdFSopendir(const char *name GT_UNUSED)
{
    int fd;

    /* look for new fd */
    for (fd = 0; fd < CMD_FS_MAX_OPEN_FILES; fd++)
    {
        if (FD.flags == 0)
        {
            break;
        }
    }
    if (fd >= CMD_FS_MAX_OPEN_FILES)
    {
        last_error = "No enough descriptors";
        return -1;
    }
    /* unused file descriptor found. */
    FD.flags = CMD_FS_O_DIRECTORY;
    FD.inode = cmdFSdir;
    return fd;
}

/**
* @internal cmdFSclosedir function
* @endinternal
*
* @brief   close a directory
*/
static int cmdFSclosedir(int fd)
{
    return cmdFSclose(fd);
}


/**
* @internal cmdFSreaddir function
* @endinternal
*
* @brief   read a directory entry
*/
static int cmdFSreaddir(int fd, GEN_FS_DIRENT_STC *dirPtr)
{
    CMD_FS_CHECK_FD();

    if ((FD.flags & CMD_FS_O_DIRECTORY) == 0)
    {
        last_error = "descriptor is not a directory";
        return -1;
    }
    if (dirPtr == NULL)
    {
        last_error = "Bad parameter: dirPtr";
        return -1;
    }
    if (FD.inode == NULL)
    {
        return 0;
    }
    CMD_FS_BZERO(dirPtr, sizeof(*dirPtr));
    CMD_FS_STRNCPY(dirPtr->d_name, FD.inode->name, GEN_FS_NAME_MAX);
    FD.inode = FD.inode->next;
    return 1;
}

/* genFS definition */
genFS_STC cmdFS = {
    cmdFSlastError,
    cmdFSopen,
    cmdFSclose,
    cmdFSread,
    cmdFSwrite,
    cmdFSlseek,
    cmdFSstat,
    cmdFSunlink,
    cmdFSopendir,
    cmdFSclosedir,
    cmdFSreaddir
};

#ifdef TEST
int main()
{
    int fd;
    char buf[100];
    int k;

    fd = cmdFSopen("test", GEN_FS_O_WRONLY); /* failure */
    fd = cmdFSopen("test", GEN_FS_O_WRONLY|GEN_FS_O_CREAT);
    k = cmdFSwrite(fd, "12345", 5);
    k = cmdFSwrite(fd, "67890abc", 8);
    cmdFSclose(fd);
    fd = cmdFSopen("test", GEN_FS_O_RDWR);
    k = cmdFSread(fd, buf, 3);
    k = cmdFSwrite(fd, "xyzkl", 5);
    k = cmdFSread(fd, buf, 10); /* 3 bytes read */
    k = cmdFSread(fd, buf, 10); /* 0 bytes: eof */
    cmdFSclose(fd);
    return 0;
}
#endif

