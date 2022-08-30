
// #include <FreeRTOS.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <cpss/generic/ipc/mvShmIpc.h>

/* Located in the CM3 SRAM reserved area */
#define IPC_SIZE            0x400
#define IPC_MAX_MSG_SIZE    0x50

static IPC_SHM_STC shm;

int ipcInit(void* ipcBase);

static void wr_Ipc_sync(
    void* cookie,
    IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    void*   ptr,
    IPC_UINTPTR_T targetPtr,
    IPC_U32 size
)
{
    IPC_U32 i;
    (void)cookie;
    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E)
    {
#if defined(IPC_CACHE_WB_INVALIDATE)
        armv7_dcache_wbinv_range(targetPtr, size);
#endif
        for (i = 0; i < size; i += 4)
        {
            *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i)) = *((volatile IPC_U32*)(targetPtr+i));
        }
        return;
    }
    /* mode == IPC_SHM_SYNC_FUNC_MODE_WRITE_E */
    for (i = 0; i < size; i += 4)
    {
        *((volatile IPC_U32*)(targetPtr+i)) = *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i));
    }
#if defined(IPC_CACHE_WB_INVALIDATE)
    armv7_dcache_wbinv_range(targetPtr, size);
#endif
}

int ipcInit(void* ipcBase)
{
    shmIpcInit( &shm, ipcBase, IPC_SIZE, 0 /* slave */, wr_Ipc_sync, NULL);

    return 0;
};

/* Shared with appl */
struct IPC_MSG {
    unsigned int msg_num;
    char    data[IPC_MAX_MSG_SIZE-4];
};


void call_ipc(char* ipcBase, char* str)
{
    struct IPC_MSG msg = {0};
    int size, i, rc;

    ipcInit(ipcBase);

    msg.msg_num = 1;
    strcpy(msg.data, str);
    rc = shmIpcSend(&shm, 1, (const void*)&msg, sizeof(msg));
    if (rc) {
        printf("shmIpcSend failed %d\n", rc);
        return;
    }

    for (i=0; i<1000; i++) {
        usleep(10000);   /* sleep for 10ms */
        if (shmIpcRxChnReady(&shm, 1)) {
            memset(&msg, 0, sizeof(msg));
            size = sizeof(struct IPC_MSG);
            if (shmIpcRecv(&shm, 1, &msg, &size)==1)
                printf("Appl got response #%d: %s\n", msg.msg_num, msg.data);
            else
                printf("Failed to get response on channel 1\n");

            return;
        }
    }

    printf("Failed to get response on channel 1\n");
}



