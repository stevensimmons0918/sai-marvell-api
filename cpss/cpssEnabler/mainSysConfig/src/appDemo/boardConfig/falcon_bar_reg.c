/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file falcon_bar_reg.c
*
* @brief A file to allow 2 functions: read/write to bar0 or bar2.
*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
*        when access to BAR0 caller give the address (offset) within the BAR0.
*        when access to BAR2 caller give the 'Cider' address within the switch.
*
* @version   1
********************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif


#ifndef ASIC_SIMULATION
#include <stdio.h>
FILE *fdopen(int fd, const char *mode);
int close(int fd);
/*#include <unistd.h>*/
#include <stdbool.h>
#include <sys/mman.h>
#else
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <asicSimulation/wmApi.h>
#endif

#define USE_DELAY
#ifdef USE_DELAY
#define MS_SLEEP sleep_ms(1);
#else
#define MS_SLEEP
#endif

typedef unsigned int GT_STATUS;
extern GT_STATUS osTimerWkAfter(uint32_t mils);
#define sleep_ms osTimerWkAfter

/* to simplify code bus,dev,func*/
#define BDF pciBus, pciDev, pciFunc
#define BDF_DECL \
    uint32_t  pciBus, \
    uint32_t  pciDev, \
    uint32_t  pciFunc

static uint32_t pciBus  = 0x0;
static uint32_t pciDev  = 0;
static uint32_t pciFunc = 0;

#ifndef ASIC_SIMULATION

static int sysfs_pci_open(
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
    printf("Resources: %s\n", fname);
    *fd = open(fname, flags);
    if (*fd < 0)
    {
        perror(fname);
        return -1;
    }
    return 0;
}

static int sysfs_pci_map(
    const char *res_name,
    int flags,
    int   *fd,
    void **vaddr
)
{
    int rc;
    struct stat st;

    rc = sysfs_pci_open(BDF, res_name, flags, fd);
    if (rc != 0)
    {
        perror(res_name);
        return rc;
    }

    if (fstat(*fd, &st) < 0)
    {
        close(*fd);
        return -1;
    }
    *vaddr = mmap(NULL,
                st.st_size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                *fd,
                (off_t)0);
    if (MAP_FAILED == *vaddr)
    {
        perror("mmap");
        close(*fd);
        return -1;
    }
    printf("%s mapped to %p, size=0x%x\n", res_name, *vaddr, (unsigned)st.st_size);
    return 0;
}
#endif /*!ASIC_SIMULATION*/

static void sysfs_pci_write(void* vaddr, int offset, uint32_t value)
{
    /*printf("Write 0x%08x to offset 0x%x\n", (uint32_t)value, offset);*/
#ifdef  ASIC_SIMULATION
    ASIC_SIMULATION_WriteMemory((uint32_t)(GT_UINTPTR)vaddr, offset, 1, &value , (GT_UINTPTR)vaddr+offset);
#else
    *((volatile uint32_t*)(((uintptr_t)vaddr)+offset)) = (uint32_t) value;
#endif
}

static uint32_t sysfs_pci_read(void* vaddr, int offset)
{
    /*printf("Read from offset 0x%x\n",  offset);*/
#ifdef  ASIC_SIMULATION
    uint32_t  data;
    ASIC_SIMULATION_ReadMemory((uint32_t)(GT_UINTPTR)vaddr, offset, 1, &data , (GT_UINTPTR)vaddr+offset);
    return data;
#else
    return(*((volatile uint32_t*)(((uintptr_t)vaddr)+offset)));
#endif
}

#define READ_OPER    1
#define WRITE_OPER   0

int falcon_bar_reg_main(
    uint32_t IN_pciBus  ,
    uint32_t IN_pciDev  ,
    uint32_t IN_pciFunc ,
    uint32_t barNum  ,
    uint32_t regAddr ,
    uint32_t regData ,/* write value on 'WRITE_OPER' */
    uint32_t oper
)
{
#ifndef  ASIC_SIMULATION
    int fd;
    FILE *f;
    int i = 0;
    unsigned long long start, end, flags;
    int rc;

    int bar0_fd;
    int bar2_fd;


    /* sysfs_read_resource physical*/
    unsigned long long res0=0, res2=0;
    void *bar0_space_ptr=NULL;
    void *bar2_space_ptr=NULL;
#else /*ASIC_SIMULATION*/
    /* sysfs_read_resource physical*/
    unsigned long long res2;
    void *bar0_space_ptr=NULL;
    void *bar2_space_ptr=NULL;
    uint32_t   regVal;
#endif /*ASIC_SIMULATION*/

    if(oper == WRITE_OPER)
    {
        printf("WRITE:pciBus[0x%x],pciDev[0x%x],pciFunc[0x%x],barNum[%d],regAddr[0x%8.8x],value[0x%8.8x]\n",
        IN_pciBus  ,
        IN_pciDev  ,
        IN_pciFunc ,
        barNum  ,
        regAddr ,
        regData );
    }
    else
    {
        printf("READ:pciBus[0x%x],pciDev[0x%x],pciFunc[0x%x],barNum[%d],regAddr[0x%8.8x]\n",
        IN_pciBus  ,
        IN_pciDev  ,
        IN_pciFunc ,
        barNum  ,
        regAddr );
    }

    pciBus = IN_pciBus;
    pciDev = IN_pciDev;
    pciFunc = IN_pciFunc;

    if(barNum != 0 && barNum != 2)
    {
        printf("BAR[%d] is not supported (only 0,4 supported) \n",barNum);
        return 1;
    }


#ifndef  ASIC_SIMULATION
    rc = sysfs_pci_open(BDF, "resource", O_RDONLY, &fd);
    /* printf("%s: %d\n", __FUNCTION__, __LINE__);*/
    if (rc != 0)
        return rc;

    f = fdopen(fd, "r");
    if (f == NULL)
        return -1;
    while (!feof(f))
    {
        if (fscanf(f, "%llu %llu %llu", &start, &end, &flags) != 3)
            break;
        if (i == 0)
            res0 = start;
        if (i == 2)
            res2 = start;
        i++;
    }
    fclose(f);

    printf("res0 (Bar 0 addr): %llx\n", res0);
    printf("res2 (Bar 2 addr): %llx\n", res2);

    rc = sysfs_pci_map("resource0", O_RDWR, &bar0_fd, &bar0_space_ptr);
    if (rc != 0)
        return rc;

    /* file descriptor is not used here */
    close(bar0_fd);

    rc = sysfs_pci_map("resource2", O_RDWR, &bar2_fd, &bar2_space_ptr);
    if (rc != 0)
    {
        return rc;
    }


    /* file descriptor is not used here */
    close(bar2_fd);

#else /* ASIC_SIMULATION */
    wmMemPciConfigSpaceRead(pciBus,pciDev,pciFunc,0x10,&regVal);
    bar0_space_ptr = (void*)(GT_UINTPTR)regVal;
    wmMemPciConfigSpaceRead(pciBus,pciDev,pciFunc,0x18,&regVal);
    bar2_space_ptr = (void*)(GT_UINTPTR)regVal;

    #if __WORDSIZE == 64
    wmMemPciConfigSpaceRead(pciBus,pciDev,pciFunc,0x14,&regVal);
    bar0_space_ptr = (void*)((GT_UINTPTR)bar0_space_ptr + (((GT_UINTPTR)regVal)<<32));

    wmMemPciConfigSpaceRead(pciBus,pciDev,pciFunc,0x1c,&regVal);
    bar2_space_ptr = (void*)((GT_UINTPTR)bar2_space_ptr + (((GT_UINTPTR)regVal)<<32));
    #endif

    res2 = (GT_UINTPTR)bar2_space_ptr;

    printf("bar0_space_ptr=0x%p \n",bar0_space_ptr);
    printf("bar2_space_ptr=0x%p \n",bar2_space_ptr);
#endif /* ASIC_SIMULATION */

    if( 0 == barNum )
    {
        if (oper == WRITE_OPER)
        {
            sysfs_pci_write(bar0_space_ptr, regAddr, regData);
        }
        else
        {
            regData = sysfs_pci_read(bar0_space_ptr, regAddr);
            printf("0x%08x\n", regData);
        }
    }
    else
    {
/* PEX ATU (Address Translation Unit) registers */
#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200
#define ATU_REGION_CTRL_1_REG         0x100
#define ATU_REGION_CTRL_2_REG         0x104
#define ATU_LOWER_BASE_ADDRESS_REG    0x108
#define ATU_UPPER_BASE_ADDRESS_REG    0x10C
#define ATU_LIMIT_ADDRESS_REG         0x110
#define ATU_LOWER_TARGET_ADDRESS_REG  0x114
#define ATU_UPPER_TARGET_ADDRESS_REG  0x118
        /* create mapping window : ATU_LOWER_BASE_ADDRESS_REG */
        sysfs_pci_write(bar0_space_ptr, 0x1308, (res2 & 0xFFFFFFFF) );
        MS_SLEEP;                /*ATU_UPPER_BASE_ADDRESS_REG*/
        sysfs_pci_write(bar0_space_ptr, 0x130c, ((res2>>32) & 0xFFFFFFFF) );
        MS_SLEEP;                /*ATU_LIMIT_ADDRESS_REG : set size to 1M (20 bits) offset from the 'base' */
        sysfs_pci_write(bar0_space_ptr, 0x1310, ((res2+0xFFFFF) & 0xFFFFFFFF) );
        MS_SLEEP;                /*ATU_REGION_CTRL_1_REG : type of region to be mem */
        sysfs_pci_write(bar0_space_ptr, 0x1300, 0x0);
        MS_SLEEP;                /*ATU_REGION_CTRL_2_REG : enable the region */
        sysfs_pci_write(bar0_space_ptr, 0x1304, 0x80000000);
        MS_SLEEP;

        /* Configure the window map : ATU_LOWER_TARGET_ADDRESS_REG*/
        sysfs_pci_write(bar0_space_ptr, 0x1314, (regAddr & 0xfff00000));
        /*{int tmp; __asm__ __volatile__("cpuid" : "=a" (tmp) : "0" (1) : "ebx", "ecx", "edx", "memory"); }*/
        MS_SLEEP;


        if (oper == WRITE_OPER)
        {
            sysfs_pci_write(bar2_space_ptr, (regAddr & 0x000fffff), regData);
        }
        else /* read operation */
        {
            regData = sysfs_pci_read(bar2_space_ptr, (regAddr & 0x000fffff));
            printf("0x%08x\n", regData);
        }
    }

    return 0;
}

/*
* read from bar0 or bar2.
*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
*        when access to BAR0 caller give the address (offset) within the BAR0.
*        when access to BAR2 caller give the 'Cider' address within the switch.*/
int falcon_bar_reg_read(
    uint32_t pciBus  ,
    uint32_t pciDev  ,
    uint32_t pciFunc ,
    uint32_t barNum  ,
    uint32_t regAddr
)
{
    return falcon_bar_reg_main(pciBus,pciDev,pciFunc,barNum,regAddr,0,READ_OPER);
}
/*
* write to bar0 or bar2.
*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
*        when access to BAR0 caller give the address (offset) within the BAR0.
*        when access to BAR2 caller give the 'Cider' address within the switch.*/
int falcon_bar_reg_write(
    uint32_t pciBus  ,
    uint32_t pciDev  ,
    uint32_t pciFunc ,
    uint32_t barNum  ,
    uint32_t regAddr ,
    uint32_t writeValue
)
{
    return falcon_bar_reg_main(pciBus,pciDev,pciFunc,barNum,regAddr,writeValue,WRITE_OPER);
}

static off_t mvDmaDrvOffset = 0;
#ifndef ASIC_SIMULATION

#include <termios.h>
#include <sys/types.h>
#include <unistd.h>

static void*  prvExtDrvDmaPtr = NULL;
static uintptr_t prvExtDrvDmaPhys = 0;
static uint64_t prvExtDrvDmaPhys64 = 0;
static uint32_t prvExtDrvDmaLen = 0;
#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

/* The offset parameter of mmap for mvDmaDrv
 * if non-zero then map DMA for PCI device
 */
#ifdef MTS_BUILD
#  define LINUX_VMA_DMABASE       0x19000000
#  define DMA_LEN                 (4*1024*1024)
#else
#  define DMA_LEN                 (2*1024*1024)
#endif

static void try_map_mvDmaDrv(void)
{
    int fd;

    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);

    fd = open("/dev/mvDmaDrv", O_RDWR);
    if (fd >= 0)
    {
        lseek(fd, mvDmaDrvOffset, 0);
        prvExtDrvDmaLen = DMA_LEN;
        prvExtDrvDmaPtr = mmap(
#if !defined(SHARED_MEMORY) && !defined(MTS_BUILD)
            NULL,
#else /* SHARED_MEMORY */
            NULL,/*(void*)((GT_UINTPTR)LINUX_VMA_DMABASE),*/
#endif
            prvExtDrvDmaLen,
            PROT_READ | PROT_WRITE,
#if defined(SHARED_MEMORY) || defined(MTS_BUILD)
            MAP_FIXED |
#endif
            MAP_32BIT | MAP_SHARED,
            fd, 0);
        if (prvExtDrvDmaPtr == MAP_FAILED)
        {
            perror("mmap(mvDmaDrv)");
            prvExtDrvDmaPtr = NULL;
            close(fd);
            return;
        }
        else
        {
            printf("mvDmaDrv mapped to %p\n",prvExtDrvDmaPtr);
        }
    }
    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);


    prvExtDrvDmaPhys = 0;
    prvExtDrvDmaPhys64 = 0;
    if (read(fd, &prvExtDrvDmaPhys64, 8) == 8)
    {
#ifdef CPU_BE
            prvExtDrvDmaPhys = __builtin_bswap64(prvExtDrvDmaPhys64);
#else
            prvExtDrvDmaPhys = (uintptr_t)prvExtDrvDmaPhys64;
#endif
    }

    close(fd);
}

/**
* @internal check_dma function
* @endinternal
*
* @brief   Check if DMA block already allocated/mapped to userspace
*         If no then allocate/map
*/
static void check_dma(void)
{
    if (prvExtDrvDmaPtr != NULL)
        return;
    /* try to map from mvDmaDrv */
    try_map_mvDmaDrv();
#if defined(MAP_HUGETLB) && !defined(SHARED_MEMORY)
    if (prvExtDrvDmaPtr == NULL)
    {
        /* try to allocate hugetlb */
        /*alloc_hugetlb();*/
    }
#endif
    if (prvExtDrvDmaPtr == NULL)
    {
        prvExtDrvDmaLen = 0;
        return;
    }
    printf("dmaLen=%d MB\n", prvExtDrvDmaLen>>20);
    printf("dmaPhys64=0x%lx\n", prvExtDrvDmaPhys64);
    printf("dmaPhys=%p\n", (void*)prvExtDrvDmaPhys);
    printf("prvExtDrvDmaPtr=%p\n", prvExtDrvDmaPtr);
}

/*
* read from DMA.
*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
*        the offset from start of DMA and should be 0x4 aligned
*        value read as 32 bits value.
*/
int debug_DMA_read(uint32_t offset)
{
    uint32_t value;
    if(prvExtDrvDmaPtr == NULL)
    {
        check_dma();
    }
    if(offset >= prvExtDrvDmaLen)
    {
        printf("error :offset is limited to [%d] MB",prvExtDrvDmaLen>>20);
        return 1;
    }

    if(prvExtDrvDmaPtr == NULL)
    {
        printf("init error\n");
        return 1;
    }

    value = *((volatile uint32_t*)prvExtDrvDmaPtr + (offset/0x4));

    printf("DMA-READ : offset[0x%x] with value [0x%x] \n",offset,value);
    return 0;
}
/*
* write to DMA.
*        IMPORTANT : this access NOT uses CPSS/AppDemo settings.
*        the offset from start of DMA and should be 0x4 aligned
*        value write as 32 bits value.
*/
int debug_DMA_write(uint32_t offset,uint32_t value)
{
    if(prvExtDrvDmaPtr == NULL)
    {
        check_dma();
    }
    if(offset >= prvExtDrvDmaLen)
    {
        printf("error :offset is limited to [%d] MB",prvExtDrvDmaLen>>20);
        return 1;
    }

    if(prvExtDrvDmaPtr == NULL)
    {
        printf("init error\n");
        return 1;
    }

    printf("DMA-write : offset[0x%x] with value [0x%x] \n",offset,value);

    *((volatile uint32_t*)prvExtDrvDmaPtr + (offset/0x4)) = value;

    return 0;
}
#else /* ! ASIC_SIMULATION */
int debug_DMA_write(uint32_t offset,uint32_t value)
{
    printf("DMA-write : NOT IMPLEMENTED for ASIC_SIMULATION : offset[0x%x] with value [0x%x] \n",offset,value);
    return 0;
}
int debug_DMA_read(uint32_t offset)
{
    printf("DMA-READ : NOT IMPLEMENTED for ASIC_SIMULATION : offset[0x%x] \n",offset);
    return 0;
}
#endif /* ASIC_SIMULATION */
/* needed for the 'first' opening of the DMA , and the place in the file : ("/dev/mvDmaDrv") */
/* needed for the 'first' opening of the DMA , and the place in the file : ("/dev/mvDmaDrv") */
int debug_DMA_print_pci(void)
{
    printf("mvDmaDrvOffset = 0x%lx \n",mvDmaDrvOffset);
    return 0;
}
int debug_DMA_set_pci_params(
    uint32_t  pciDomain,
    uint32_t  pciBus,
    uint32_t  pciDev,
    uint32_t  pciFunc
)
{
#ifndef ASIC_SIMULATION
    if(prvExtDrvDmaPtr != NULL)
    {
        /* there is no meaning to call this function now ... */
        printf("the DMA params must be called BEFORE any DMA access and 'first' DMA allocation \n");
        printf("Error : ignoring the parameters \n");
    }
#endif /*ASIC_SIMULATION*/
    /***************************************************/
    /** pre-Configure DMA                             **/
    /** Configure parameter for mvDmaDrv              **/
    /**                                               **/
    /** mvDmaDrv will allocate DMA memory for this    **/
    /** PCI device. It will also configure IOMMU,     **/
    /** so DMA transactions from PP to system memory  **/
    /** will be allowed                               **/
    /***************************************************/
    mvDmaDrvOffset =
            ((pciDomain & 0xffff) << 16) |
            ((pciBus & 0xff) << 8) |
            ((pciDev & 0x1f) << 3) |
            (pciFunc & 0x7);

    debug_DMA_print_pci();
    return 0;
}


