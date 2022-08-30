
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <i2c_lib.h>
#include <mvTwsi.h>

extern void call_ipc(char* ipcBase, char* str);

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
/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define MV_BYTE_SWAP_32BIT(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))

/* to simplify code bus,dev,func*/
#define BDF pciBus, pciDev, pciFunc
#define BDF_DECL \
    uint32_t  pciBus, \
    uint32_t  pciDev, \
    uint32_t  pciFunc

#define _2K     2048

#define DEFAULT_CM3_OFFSET              0x80000
#define DEFAULT_CM3_SIZE                0x20000
#define CETUS_CM3_OFFSET                0x40000
#define CETUS_CM3_SIZE                  0x1F800
#define AC5_MG_MEMORY_SPACE_SIZE        0x00100000
#define NUM_OF_CM3_IN_AC5               3
#define AC5_MG_BASE_ADDRESS             0x7F900000
#define FALCON_TILE0_MG_BASE_ADDRESS    0x1D000000
#define FALCON_TILE2_MG_BASE_ADDRESS    0x5D000000
#define FALCON_MG_MEMORY_SIZE           0x00100000
#define NUM_OF_CM3_IN_FALCON_DUAL_TILE  2
#define AC5P_MG_BASE_ADDRESS            0x3C200000
#define FALCON_CM3_SIZE                 0x60000

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

uint32_t direct_0_or_through_pci_1;

static void pp_write_reg(int32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(pp_space_ptr + offset) = value;
}

static uint32_t pp_read_reg(int32_t offset)
{
    return *(volatile uint32_t *)(pp_space_ptr + offset);
}


static void enable_cm3(unsigned int dev_id, bool enable)
{
    uint32_t reg;

    reg = pp_read_reg(0x500);
    if (enable) {
        reg |= (1 << 28); /* Enable CM3 */
        if (0xbe00 != dev_id) {
            reg |= (1 << 29); /* Enable CM3 */
        } else {
            reg |= (1 << 19); /* bit 19: CM3_PROC_CLK_DIS */
        }
    } else {
        reg &= ~(1 << 28); /* disable CM3 */
        if (0xbe00 != dev_id) {
            reg &= ~(1 << 29); /* disable CM3 */
        } else {
            reg |= 0x00100000; /* bit 20: init ram */
            reg &= 0xfff7ffff; /* bit 19: CM3_PROC_CLK_DIS */
        }
    }

    pp_write_reg(0x500, reg);

    if (0xbe00 == dev_id && !enable) {
        reg = pp_read_reg(0x54);
        reg |= 1; // bit 0: CM3_METAL_FIX_JTAG_EN
        pp_write_reg(0x54, reg);
    }
}

int sysfs_pci_open(
    BDF_DECL,
    const char *name,
    int     flags,
    int     *fd
)
{
    char fname[128];
    FILE *f;
    if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
        return -1;

    sprintf(fname, "sys/bus/pci/devices/0000:%02x:%02x.%x/enable",pciBus, pciDev, pciFunc);
    f = fopen(fname, "w");
    fputs("1",f);
    fclose(f);

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

int eeprom_write_2_words(unsigned char i2c_addr, int offset, uint32_t data0, uint32_t data1) {
    char buffer[8];

    *(uint32_t *)(buffer) = MV_BYTE_SWAP_32BIT(data0);
    *(uint32_t *)(buffer + 4) = MV_BYTE_SWAP_32BIT(data1);
    if( 0 == direct_0_or_through_pci_1 ) {
        if (0 < i2c_transmit_with_offset(i2c_addr, 8, 0, i2c_offset_type_16_E, offset, buffer)) {
            printf("w 0x%08x 0x%08x\n", data0, data1);
            return 0;
        } else {
            printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
            return -1;
        }
    }
    else /* 1 == direct_0_or_through_pci_1 */
    {
        MV_TWSI_SLAVE mv_twsi_slave;

        mv_twsi_slave.slaveAddr.address = i2c_addr << 1;
            mv_twsi_slave.slaveAddr.type = ADDR7_BIT;
            mv_twsi_slave.validOffset = 1;
            mv_twsi_slave.offset = offset;
            mv_twsi_slave.offset_length = 2;

        if ( MV_OK == mv_twsi_master_transceive( buffer, 8, &mv_twsi_slave, MV_TWSI_TRANSMIT) ) {
            printf("w 0x%08x 0x%08x\n", data0, data1);
            return 0;
        } else { /* MV_FAIL */
            printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
            return -1;
        }
    }
}

int main(int argc, char *argv[])
{
    char copy_buf[256];
    struct stat fw_stat;
    uint32_t reg;

    int fd;
    FILE *f;
    int i = 0, offset;
    unsigned long long start, end, flags;
    int rc;

    int fw_fd = 0, xbar_fd, mg_fd;
    uint32_t *cm3_sram_ptr = NULL, *xbar_space_ptr = NULL;

    // sysfs_read_resource physical
    unsigned long long res2, res4;
    uint8_t i2c_addr;
    uint32_t dev_id;
    uint32_t cm3_id;

    uint32_t cm3_offset = DEFAULT_CM3_OFFSET;
    uint32_t cm3_size = DEFAULT_CM3_SIZE;

    void *buffer;
    uint32_t *ptr;

    uint32_t sram_base;
    enum device_type dev_type;
    uint32_t valid_input = 1;

    /* Check inputs */
    if (argc >= 4)
    {
        cm3_id = strtoul(argv[2], NULL, 0);
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
            cm3_size = CETUS_CM3_SIZE;
        }
        else if (!strcmp(argv[1], "falcon"))
        {
            if (cm3_id < 16) {
                printf("cm3 ID 0..15 (Ravens) are not supported yet\n");
                return -1;
            }
            cm3_id -= 16;
            dev_type = DEVICE_TYPE_FALCON_E;
            sram_base = (cm3_id > 1) ? FALCON_TILE2_MG_BASE_ADDRESS : FALCON_TILE0_MG_BASE_ADDRESS;
            /* each tile has 4 MG units where only one MG has CM3 unit
             */
            sram_base += ((cm3_id % NUM_OF_CM3_IN_FALCON_DUAL_TILE)*4) * FALCON_MG_MEMORY_SIZE;
            cm3_size = FALCON_CM3_SIZE;
        }
        else
            valid_input = 0;
    }
    else {
        valid_input = 0;
    }


    if ( !valid_input )
    {
        printf("Usage:\n");
        printf("       dev: device type - ac3, ac5ex, ac5in, ac5p, aldrin, falcon\n");
        printf("       cm3: cm3 instance id, range [0..19] , depends on device type\n\n");
        printf("       %s <dev> <cm3> <file> - load FW from binary image file to CM3 SRAM and run it\n", argv[0]);
        printf("       %s <dev> <cm3> -i <string> - sends IPC message to FW\n", argv[0]);
        printf("       %s <dev> -w <offset> <value> - write PP register\n", argv[0]);
        printf("       %s <dev> -r <offset> - print PP register\n", argv[0]);
        printf("       %s <dev> -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", argv[0]);
        printf("       %s <dev> -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", argv[0]);
        return 0;
    }


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

        // Config Aldrin PeX Window 0 to see Switching-Core
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
        cm3_sram_ptr =(uint32_t*)(pp_space_ptr) + 0x80000/sizeof(uint32_t);
        *(unsigned*)(pp_space_ptr + 0x124) = 0x1;
    }
    dev_id = (pp_read_reg(0x4c) >> 4) & 0xffff;

    /* Set 8 region completion register mode */
    reg = pp_read_reg(COMP_REG_CTRL_REG);
    reg &= ~(1 << 16);
    pp_write_reg(COMP_REG_CTRL_REG, reg);

    /* PP write command */
    if (!strcmp(argv[2], "-w")) {
        pp_write_reg(strtoul(argv[3], NULL, 0), strtoul(argv[4], NULL, 0));
        return 0;
    }

    /* PP read command */
    if (!strcmp(argv[2], "-r")) {
        printf("0x%08x\n", pp_read_reg(strtoul(argv[3], NULL, 0)));
        return 0;
    }

    if (!strcmp(argv[3], "-i")) {
        call_ipc((char*)cm3_sram_ptr + cm3_size - _2K, argv[4]);
        return 0;
    }


    /* EEPROM program command */
    if (!strcmp(argv[2], "-e") || !strcmp(argv[2], "-p")) {

        if (!strcmp(argv[2], "-e")) {
            direct_0_or_through_pci_1 = 0;
        } else { /* (!strcmp(argv[2], "-p")) */
            printf("Don't forget to disable EEPROM\n");
            direct_0_or_through_pci_1 = 1;
        }

        if (argc < 5) {
            if (0 == direct_0_or_through_pci_1) {
                printf("Usage: %s <dev> -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", argv[0]);
            } else { /* (1 == direct_0_or_through_pci_1) */
                printf("Usage: %s <dev> -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", argv[0]);
            }
            return -1;
        }

        i2c_addr = strtoul(argv[3], NULL, 0);

        fw_fd = open(argv[4], O_RDONLY);
        if (fw_fd <= 0) {
            fprintf(stderr, "Cannot open %s file.\n", argv[4]);
            return -1;
        }

        if (0 == direct_0_or_through_pci_1) {
            i2c_init();
            if (0 < i2c_receive_with_offset (i2c_addr, 0x20, 0, i2c_offset_type_16_E, 0, copy_buf)) {
                printf("i2c read from %x offset 0: 0x%08x\n", i2c_addr, *(uint32_t *)copy_buf);
            } else {
                printf("i2c failed\n");
            }
        } else { /* (1 == direct_0_or_through_pci_1) */
            uint32_t mv_rc;
            mv_rc = mv_twsi_init(100000, 250000000 /*250MHZ*/);
            if ( 90000 < mv_rc ) {
                printf("mv_twsi_init passed, actual freq %d\n", mv_rc);
            } else {
                printf("mv_twsi_init failed, actual freq %d\n", mv_rc);
                return -1;
            }
        }

        i = 0x30080002;
        offset = 0;
        eeprom_write_2_words(i2c_addr, offset, 0x8001320c, 0x00010000);
        while (true) {
            rc = read(fw_fd, &reg, 4);
            offset += 8;
            if (rc < 4) {  /* Handle last bytes of file */
                while (rc < 4)
                    *(uint8_t *)(&reg + rc++) = 0;
                rc = 0;
            }
            eeprom_write_2_words(i2c_addr, offset, i, reg);
            if (rc < 4)
                break;
            i += 4;
        }
        eeprom_write_2_words(i2c_addr, offset + 8,  0x30000502, 0x300e1a80); /* Take CM3 out of reset */
        eeprom_write_2_words(i2c_addr, offset + 16, 0xffffffff, 0xffffffff); /* Mark end of data */

        return 0;
    }


    /* File load command */
    fw_fd = open(argv[3], O_RDONLY);
    if (fw_fd <= 0) {
        fprintf(stderr, "Cannot open %s file.\n", argv[3]);
        return -1;
    }

    enable_cm3(dev_id, 0); /* Disable CM3 */

    fstat(fw_fd, &fw_stat);
    if (fw_stat.st_size > cm3_size) {
        printf("Error - file too large (%ld), we have only %dKB space\n", fw_stat.st_size, cm3_size/1024);
        close(fw_fd);
        return -1;
    }

    buffer = malloc(fw_stat.st_size);
    if (buffer == NULL)
        return -1;

    ptr = buffer;
    read(fw_fd, buffer, fw_stat.st_size);
    while ((void*)ptr < (buffer + fw_stat.st_size))
    {
        *cm3_sram_ptr = *ptr;
        ptr++;
        cm3_sram_ptr++;
    }

    free(buffer);
    close(fw_fd);

    printf("successfully loaded file %s, size %ld\n", argv[3], fw_stat.st_size);

    enable_cm3(dev_id, 1); /* Enable CM3 */

    if( dev_type == DEVICE_TYPE_AC5_INTERNAL_E )
    {
        munmap(pp_space_ptr, 0x100000);
        close(fd);
    }

    return 0;
}
