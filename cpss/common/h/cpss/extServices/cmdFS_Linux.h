
#ifndef __cmdFS_Linux_h__
#define __cmdFS_Linux_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GEN_FS_O_RDONLY 0x01
#define GEN_FS_O_WRONLY 0x02
#define GEN_FS_O_RDWR   0x03
#define GEN_FS_O_CREAT  0x04
#define PERMS           0666

struct genFS_STC_Linux {
    const char* (*lastError)(void);
    int (*open)(const char* name, int flags);
    int (*close)(int fd);
    int (*read)(int fd, void *buf, int count);
    int (*write)(int fd, const void *buf, int count);
    int (*lseek)(int fd, int offset, int whence);
    int (*stat)(const char* name, void *buf);
    int (*unlink)(const char* name);
    int (*opendir)(const char *name);
    int (*closedir)(int fd);
    int (*readdir)(int fd, void *dirPtr);
};

typedef struct genFS_STC_Linux genFS_STC_Linux;
typedef struct genFS_STC_Linux *genFS_PTR_Linux;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

