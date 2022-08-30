
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

/* Demo app includes. */
#include <printf.h>
#include <cpss/generic/ipc/mvShmIpc.h>
#include "global.h"


/* Located in the SRAM reserved area */
#define IPC_SIZE            0x400
#define IPC_MAX_MSG_SIZE    0x50

static IPC_SHM_STC shm;

static void prvIpcTask(void *pvParameters);

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


/**
* @internal srvCpuIpcInit function
* @endinternal
*
* @brief  Initialize IPC struct, creates IPC task
*
* @param[in] ipcBase            - virtual address of shared memory block
*/
void srvCpuIpcInit(void* ipcBase)
{
    int rc;

    shmIpcInit( &shm, ipcBase, IPC_SIZE, 1 /* master */, wr_Ipc_sync, NULL);

    /* Create channel #1 with 2 buffers of 0x50 bytes for RX and for TX */
    rc = shmIpcConfigChannel(
        &shm, 1 /* chn */,
        2, IPC_MAX_MSG_SIZE,   /* rx */
        2, IPC_MAX_MSG_SIZE);  /* tx */
    if (rc) {
        printf("Failed to configure IPC: %d\n", rc);
        return;
    }

    xTaskCreate(prvIpcTask, (signed portCHAR *)"IPC",
                configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 2, NULL);
};

/* Shared with appl */
struct IPC_MSG {
    unsigned int msg_num;
    char    data[IPC_MAX_MSG_SIZE-4];
};



static void prvIpcTask(void *pvParameters)
{
    static struct IPC_MSG msg;
    static unsigned int msg_num = 0;

    int size;

    for (;;) {
        vTaskDelay(configTICK_RATE_HZ / 10);

        if (shmIpcRxChnReady(&shm, 1)) {
            size = sizeof(struct IPC_MSG);
            if (shmIpcRecv(&shm, 1, &msg, &size)==1)
            {
                msg.data[sizeof(msg.data)-1] = 0; /* Make sure the string ends with NULL */
                msg_num++;
                printf("RTOS got message #%d: %s\n", msg_num, msg.data);
                size = sizeof(struct IPC_MSG);
                strcpy(msg.data, "hello to you too");
                msg.msg_num = msg_num;
                if (shmIpcSend(&shm, 1, &msg, size))
                    printf("Failed to send response to message #%d\n", msg.msg_num);
            }
        }
    }
}
