#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>

#include "mvShmUart.h"
#include "common.h"

/* SHM_UART_BASE & SHM_UART_SIZE are passed from Makefile */

/***********************************************************************
 * CM3 processor tool:
 *
 *   CM3 SRAM is located at offset 0x80000 of the PP address space.
 *   PP register 0x500 controls CM3:
 *      bit [28] - CM3_Enable
 *      bit [20] - init RAM
 *      bit [19] - CM3 clock disable
 **********************************************************************/

#define COMP_REG(_x) (0x120+(_x)*4)
#define BITS19_31(a) (a & 0xfff80000)
#define BITS0_18(a) (a & 0x7ffff)
#define COMP_REG_OFF(reg) (reg<<19)
#define COMP_REG_CTRL_REG 0x140

/* to simplify code bus,dev,func*/
#define BDF pciBus, pciDev, pciFunc
#define BDF_DECL \
    uint32_t  pciBus, \
    uint32_t  pciDev, \
    uint32_t  pciFunc

#define NUM_OF_TILES                   4
#define TILE_OFFSET_OFFSET             0x20000000

#define NUM_OF_RAVENS_PER_TILE         4
#define RAVEN_MEMORY_SPACE_SIZE        0x01000000

#define DEFAULT_CM3_OFFSET              0x80000
#define DEFAULT_CM3_SIZE                0x20000
#define CETUS_CM3_OFFSET                0x40000
#define CETUS_CM3_SIZE                  0x1F800
#define AC5_MG_MEMORY_SPACE_SIZE       0x00100000
#define NUM_OF_CM3_IN_AC5              3
#define AC5_MG_BASE_ADDRESS            0x7f900000
#define FALCON_TILE0_MG_BASE_ADDRESS   0x1D000000
#define FALCON_TILE2_MG_BASE_ADDRESS   0x5D000000
#define FALCON_MG_MEMORY_SIZE          0x00100000
#define NUM_OF_CM3_IN_FALCON_DUAL_TILE 2
#define AC5P_MG_BASE_ADDRESS           0x3C200000

enum device_type {
    DEVICE_TYPE_ALDRIN_E,
    DEVICE_TYPE_PIPE_E,
    DEVICE_TYPE_BC3_E,
    DEVICE_TYPE_CETUS_E,
    DEVICE_TYPE_ALDRIN2_E,
    DEVICE_TYPE_FALCON_E,
    DEVICE_TYPE_AC5_EXTERNAL_E,
    DEVICE_TYPE_AC5_INTERNAL_E,
    DEVICE_TYPE_AC5P_E,
    DEVICE_TYPE_HARRIER_E
};

int terminal_init(void);

#ifdef WATCHDOG
volatile MV_U32  *wdExpired; /* Predefined: 1 word. Located at the end of the shared uart */
                              /* the WatchDog expired located after the RX and TX chains
                                 when receiving WatchDog interrupts, this pointer will gets MV_TRUE.
                                 The VUART tool always checks (in vuart_rx_function()) if a WD interrupt has occurred,
                                 and if so it initializes the VUART, to align the markers and continue working with VUART. */
#endif
unsigned long long cm3_shm_buffs_phys = 0;

enum device_type dev_type;
void *pp_space_ptr2;
int fd;

/*
 * PCI address is hard coded here to be 0000:00:01.0
 * which is the most common case for Prestera device
 * to be found on an embedded system. However, this may
 * require change, dynamic search or to be provided.
 */
const uint32_t pciBus = 1;
const uint32_t pciDev = 0;
const uint32_t pciFunc = 0;

void *pp_space_ptr = NULL;
void *dfx_space_ptr = NULL;
void *mg_space_ptr = NULL;

static void pp_write_reg(int32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(pp_space_ptr + offset) = value;
}

static uint32_t pp_read_reg(int32_t offset)
{
    return *(volatile uint32_t *)(pp_space_ptr + offset);
}

int sysfs_pci_open(
    BDF_DECL,
    const char *name,
    int     flags,
    int     *fd
)
{
    char fname[128];
    if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
        return -1;
    sprintf(fname, "/sys/bus/pci/devices/0000:%02x:%02x.%x/%s",
            pciBus, pciDev, pciFunc, name);
    *fd = open(fname, flags);
    if (*fd < 0) {
        perror(fname);
        return -1;
    }
    return 0;
}

int sysfs_pci_map(
    const char *res_name,
    int flags,
    int   *fd,
    void **vaddr
)
{
    int rc;
    struct stat st;

    rc = sysfs_pci_open(BDF, res_name, flags, fd);
    if (rc != 0) {
        perror(res_name);
        return rc;
    }

    if (fstat(*fd, &st) < 0) {
        close(*fd);
        return -1;
    }

    *vaddr = mmap(NULL,
                st.st_size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                *fd,
                (off_t)0);
    if (MAP_FAILED == *vaddr) {
        perror("mmap");
        close(*fd);
        return -1;
    }

    printf("%s mapped to %p, size=0x%x\n", res_name, *vaddr, (unsigned)st.st_size);

    return 0;
}

void sysfs_pci_write(void* vaddr, int offset, uint32_t value)
{
    volatile uint32_t *addr;

    addr = (uint32_t *)((uintptr_t)vaddr + offset);
    *addr = (uint32_t)value;
}

uint32_t sysfs_pci_read(void* vaddr, int offset)
{
    volatile uint32_t *addr;

    addr = (uint32_t *)((uintptr_t)vaddr + offset);
    return *addr;
}

long getRavenBaseAddr(long ravenId)
{
    long base;

    base = (ravenId/NUM_OF_RAVENS_PER_TILE) * TILE_OFFSET_OFFSET;

    if( (ravenId % 8) < 4 )
    {
    base += (ravenId % NUM_OF_RAVENS_PER_TILE) * RAVEN_MEMORY_SPACE_SIZE;
    } else {
        base += (NUM_OF_RAVENS_PER_TILE - (ravenId % NUM_OF_RAVENS_PER_TILE) - 1) * RAVEN_MEMORY_SPACE_SIZE;
    }

    return base;
}

static void *vuart_rx_function(void *sram_add)
{
    int c;
    while (1)
    {
        usleep(10000);   /* sleep for 10ms */
#ifdef WATCHDOG
        if (*wdExpired == MV_TRUE) {
            *wdExpired = MV_FALSE;
            mvShmUartInit((MV_U32*)(sram_add + SHM_UART_BASE), SHM_UART_SIZE);
        }
#endif

        while ((c=mvShmUartGetc(0)))
            putchar(c);
    }

    return NULL;
}

/* Usage: ./vuart [board type] [cm3 Id] */
int main(int argc, char* argv[])
{
    pthread_t rx_thread;
    int fd, i, mg_fd;
    FILE *f;
    int c, rc;
    void*  sram_addr = NULL;
    unsigned long long start, end, flags, res2, res4;
    uint32_t cm3_id = 0; /* for platforms that have multiple cm3 and not just one */
    uint32_t reg, sram_base;
    void *cm3_sram_ptr = NULL;
    uint32_t cm3_offset = DEFAULT_CM3_OFFSET;

    if (argc < 2)
        goto USAGE;

    if (argc > 2)
        cm3_id = strtoul(argv[2], NULL, 0);
    printf("CM3Id: %d\n", cm3_id);

    if (!strcmp(argv[1], "aldrin"))
    {
        dev_type = DEVICE_TYPE_ALDRIN_E;
    }
    else if (!strcmp(argv[1], "ac5ex"))
    {
        dev_type = DEVICE_TYPE_AC5_EXTERNAL_E;
        sram_base = AC5_MG_BASE_ADDRESS;
        sram_base += (cm3_id % NUM_OF_CM3_IN_AC5) * AC5_MG_MEMORY_SPACE_SIZE;
    }
    else if (!strcmp(argv[1], "ac5in"))
    {
        dev_type = DEVICE_TYPE_AC5_INTERNAL_E;
        sram_base = AC5_MG_BASE_ADDRESS;
        sram_base += (cm3_id % NUM_OF_CM3_IN_AC5) * AC5_MG_MEMORY_SPACE_SIZE;
    }
    else if (!strcmp(argv[1], "ac5p"))
    {
        dev_type = DEVICE_TYPE_AC5_EXTERNAL_E;
        sram_base = AC5P_MG_BASE_ADDRESS;
    }
    else if (!strcmp(argv[1], "cetus"))
    {
        dev_type = DEVICE_TYPE_CETUS_E;
        cm3_offset = CETUS_CM3_OFFSET;
    }
    else if (!strcmp(argv[1], "falcon"))
    {
        if (cm3_id < 16) {
            sram_base = (getRavenBaseAddr(cm3_id) + 0x00300000); /* cm3_id parameter is actual used as raven id */
            printf("raven %d sram_base 0x%08x\n", cm3_id, sram_base);
        } else {
            /* each tile has 4 MG units with only one MG having CM3 */
            cm3_id -= 16;
            dev_type = DEVICE_TYPE_FALCON_E;
            sram_base = (cm3_id > 1) ? FALCON_TILE2_MG_BASE_ADDRESS : FALCON_TILE0_MG_BASE_ADDRESS;
            /* each tile has 4 MG units where only one MG has CM3 unit */
            sram_base += ((cm3_id % NUM_OF_CM3_IN_FALCON_DUAL_TILE)*4) * FALCON_MG_MEMORY_SIZE;
        }
    } else
        goto USAGE;

    if ( dev_type == DEVICE_TYPE_AC5_INTERNAL_E ) /* Internal CPU - using virtual address  */
    {
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
            return -1;
        pp_space_ptr = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, sram_base);
        if (pp_space_ptr == MAP_FAILED)
        {
            printf("Could not mmap\n");
            return -1;
        }
    }
    else /* External CPU - using PCI*/
    {
        rc = sysfs_pci_open(BDF, "resource", O_RDONLY, &fd);
        if (rc != 0)
            return rc;


        f = fdopen(fd, "r");
        if (f == NULL)
            return -1;
        while (!feof(f))
        {
            if (fscanf(f, "%lli %lli %lli", &start, &end, &flags) != 3)
                break;
            if (i == 2)
                res2 = start;
            if (i == 4)
                res4 = start;
            i++;
        }
        fclose(f);

        printf("res2 (pysical PP bar 2 addr): %llx\n", res2);

        // Config Aldrin PCIe Window 0 to see Switching-Core
        rc = sysfs_pci_map("resource0", O_RDWR, &mg_fd, &mg_space_ptr);
        if (rc != 0)
            return rc;

        /* Init: create mappings for PP and XBAR */
        rc = sysfs_pci_map("resource2", O_RDWR | O_SYNC, &mg_fd, &pp_space_ptr);
        if (rc < 0)
            return -1;
    }


    if ( dev_type < DEVICE_TYPE_FALCON_E) /* Sip5 Devices */
    {
        sysfs_pci_write(mg_space_ptr, 0x41820, 0);
        sysfs_pci_write(mg_space_ptr, 0x41824, res2);
        sysfs_pci_write(mg_space_ptr, 0x41828, 0);
        sysfs_pci_write(mg_space_ptr, 0x4182C, 0);
        sysfs_pci_write(mg_space_ptr, 0x41820, 0x03ff0031);
        sysfs_pci_write(mg_space_ptr, 0x41830, 0);
        sysfs_pci_write(mg_space_ptr, 0x41834, res4);
        sysfs_pci_write(mg_space_ptr, 0x41838, 0);
        sysfs_pci_write(mg_space_ptr, 0x4183C, 0);
        sysfs_pci_write(mg_space_ptr, 0x41830, 0x001f0083);

        /* Init: create mappings for DFX */
        rc = sysfs_pci_map("resource4", O_RDWR | O_SYNC, &mg_fd, &dfx_space_ptr);
        if (rc < 0)
            return -1;

        /* Completion register */
        pp_write_reg(COMP_REG(7), htole32(BITS19_31(cm3_offset)>>19) );
        cm3_sram_ptr = pp_space_ptr + COMP_REG_OFF(7) + BITS0_18(cm3_offset)/* actually 0 */;
    }
    else  /* Sip6 Devices */
    {
        if( dev_type != DEVICE_TYPE_AC5_INTERNAL_E ) /* External CPU*/
        {
            /* configure inbound window into the PP MG0, size 1MB, at start of res2 address space */
            /* take this window because it not used by other resources */
            *(unsigned*)(mg_space_ptr + 0x1304) = 0x80000000; /* enable the window */
            *(unsigned*)(mg_space_ptr + 0x1308) = (unsigned long)res2 & 0xFFFFFFFF; /* the start of the window */
            *(unsigned*)(mg_space_ptr + 0x130c) = 0x0;
            *(unsigned*)(mg_space_ptr + 0x1310) = ((unsigned long)(res2 & 0xFFFFFFFF)) | 0xfffff; /* the end of the window (window size is 0xfffff) */
            *(unsigned*)(mg_space_ptr + 0x1314) = sram_base; /* we can accsess sram_base address through this window */
            *(unsigned*)(mg_space_ptr + 0x131c) = 0x0;
        }

        /* Completion register */
        cm3_sram_ptr = pp_space_ptr + 0x80000;
        *(unsigned*)(pp_space_ptr + 0x124) = 0x1;
    }

    /* Set 8 region completion register mode */
    reg = pp_read_reg(COMP_REG_CTRL_REG);
    reg &= ~(1 << 16);
    pp_write_reg(COMP_REG_CTRL_REG, reg);

    printf("SHM_UART_BASE: 0x%x, SHM_UART_SIZE %u\n", SHM_UART_BASE, SHM_UART_SIZE);
    printf("sram_addr: %p, cm3_sram_phys: 0x%llx\n", cm3_sram_ptr, res2 + 0x80000);
    printf("SHM UART virt addr: %p, phys: 0x%p\n", cm3_sram_ptr + SHM_UART_BASE, sram_addr + SHM_UART_BASE);

#ifdef WATCHDOG
    /* locate the watchdog expired word after the rx ant tx chains */
    wdExpired = cm3_sram_ptr + SHM_UART_BASE + 2*SHM_UART_SIZE; /* use 2 SHM_UART_SIZE, one for RX and one for TX chain */
    printf("wdExpired address: %p\n", wdExpired);
#endif
    if (terminal_init()) {
        if (errno == ENOTTY)
            fprintf(stderr, "This program requires a terminal.\n");
        else
            fprintf(stderr, "Cannot initialize terminal: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    mvShmUartInit((MV_U32*)(cm3_sram_ptr + SHM_UART_BASE), SHM_UART_SIZE);

    /* create thread to read firmware characters if exist
       if watchdog enabled, check if WD interrupt is asserted */
    rc = pthread_create(&rx_thread, NULL, vuart_rx_function, cm3_sram_ptr);
    if (rc) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",rc);
        exit(EXIT_FAILURE);
    }

    printf("Press CTRL+shift+x or ` to quit\n");

    while ((c = getc(stdin)) != EOF)
    {
        if (c==0x18 || c=='`')
            break;
        else
            mvShmUartPutc(0, c);
    }

    fprintf(stderr, "\n");  /* Can't write yet to stdout */
    fflush(NULL);

    if( dev_type == DEVICE_TYPE_AC5_INTERNAL_E )
    {
        munmap(pp_space_ptr2, 0x100000);
        close(fd);
    }

    return EXIT_SUCCESS;

USAGE:
    printf("Usage: %s <dev> <cm3>\n", argv[0]);
    printf("  dev: device type - ac3, ac5ex, ac5in, ac5p, aldrin, falcon\n");
    printf("  cm3: Optional cm3 instance id. Range 0-19, depends on device type\n");
    printf("       For Falcon, 0-15 for Ravens, 16-19 for tiles CM3\n");
    return EXIT_FAILURE;
}

