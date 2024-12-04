#include "ecnic.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <unistd.h>

#define ERROR_BODY(message) std::cout << "ERROR:\t" << __FUNCTION__ << ":\t" \
                                      << message << std::endl
#define pr_errno() ERROR_BODY(std::strerror(errno))
#define pr_errno_msg(message) ERROR_BODY(std::strerror(errno) << std::endl \
                                                              << message)
#define pr_err_msg(message) ERROR_BODY(message)
#define PRINT_ERROR                                              \
    do                                                           \
    {                                                            \
        fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
                __LINE__, __FILE__, errno, strerror(errno));     \
        exit(1);                                                 \
    } while (0)
#define SYSFS_PCI_DEVICES "/sys/bus/pci/devices"
template <typename I>
std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1)
{
    static const char *digits = "0123456789ABCDEF";
    std::string rc(hex_len, '0');
    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
        rc[i] = digits[(w >> j) & 0x0f];
    return rc;
}

std::string get_sysfs_path(const std::string &sysfs_base, int domain, int bus, int dev, int func)
{
    return sysfs_base + "/" + n2hexstr(domain, 4) + ":" + n2hexstr(bus, 2) + ":" + n2hexstr(dev, 2) + "." + n2hexstr(func, 1) + "/resource0";
}

static uint32_t read32(void *ptr, uint64_t addr)
{
    return *((volatile uint32_t *)((char *)ptr + addr));
}

static void write32(void *ptr, uint64_t addr, uint32_t data)
{
    *((volatile uint32_t *)((char *)ptr + addr)) = data;
}

static uint64_t read64(void *ptr, uint64_t addr)
{
    return *((volatile uint64_t *)((char *)ptr + addr));
}

static void write64(void *ptr, uint64_t addr, uint64_t data)
{
    *((volatile uint64_t *)((char *)ptr + addr)) = data;
}

static void mappingtest(char *resource_path)
{
    int fd;
    if ((fd = open(resource_path, O_RDWR | O_SYNC)) == -1)
    {
        PRINT_ERROR;
    }
    printf("%s opened.\n", resource_path);
    fflush(stdout);

    unsigned int map_size;
    struct stat st;
    stat(resource_path, &st); // get BAR size through sysfs file size
    map_size = st.st_size;

    void *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // 131072 map size = 128KB (131072 bytes) = 32 * 4096 bytes
    if (ptr == MAP_FAILED)
    {
        pr_errno();
        close(fd);
    }

    printf("PCI Memory mapped to user space at address %p.\n", ptr);
    fflush(stdout);

    printf("PCI BAR0 = 0x%04X\n", *((unsigned short *)ptr));

    // 寫入和讀取測試
    uint64_t val = read64(ptr, 0);
    uint64_t new_val = val+1;
    printf("Read back value at offset 0: 0x%08lX\n", read64(ptr, 0));
    printf("Read back value at offset 4: 0x%08lX\n", read64(ptr, 1));
    printf("Read back value at offset 8: 0x%08lX\n", read64(ptr, 2));

    // Step 1: 寫入示範數據
    write64(ptr, 0, new_val);
    printf("Written 0x%08lX to BAR0 offset 0.\n", new_val);

    // Step 2: 驗證讀取
    uint64_t read_val0 = read64(ptr, 0);
    printf("Read back value at offset 0: 0x%08lX\n", read_val0);

    // 驗證寫入是否正確
    if (read_val0 == new_val)
    {
        printf("Data verification successful!\n");
    }
    else
    {
        printf("Data verification failed!\n");
    }

    write64(ptr, 0, val);
    printf("Written 0x%08lX to BAR0 offset 0.\n", val);
    printf("Read back value at offset 0: 0x%08lX\n", read64(ptr, 0));

    // 解除映射
    munmap(ptr, getpagesize());
    close(fd);
}

static void *mapping(char *resource_path)
{
    printf("\nMapping.\n");
    int fd;
    if ((fd = open(resource_path, O_RDWR | O_SYNC)) == -1)
    {
        PRINT_ERROR;
    }
    printf("%s opened.\n", resource_path);
    fflush(stdout);

    unsigned int map_size;
    struct stat st;
    stat(resource_path, &st); // get BAR size through sysfs file size
    map_size = st.st_size;

    void *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // 131072 map size = 128KB (131072 bytes) = 32 * 4096 bytes
    if (ptr == MAP_FAILED)
    {
        pr_errno();
        close(fd);
    }

    printf("PCI Memory mapped to user space at address %p.\n", ptr);
    fflush(stdout);

    printf("PCI BAR0 = 0x%08X\n", *((unsigned int *)ptr));
    close(fd);
    return ptr;
}

static void readwriteTest(void *ptr)
{
    printf("\nRead/Write Test.\n");
    // 寫入和讀取測試
    uint32_t val = read32(ptr, 0);
    uint32_t new_val = 0x001C0001;
    printf("Read back value at offset 0: 0x%08X\n", read32(ptr, 0));
    printf("Read back value at offset 1: 0x%08X\n", read32(ptr, 1));
    printf("Read back value at offset 2: 0x%08X\n", read32(ptr, 2));

    // Step 1: 寫入示範數據
    write32(ptr, 0, new_val);
    printf("Written 0x%08X to BAR0 offset 0.\n", new_val);

    // Step 2: 驗證讀取
    uint32_t read_val0 = read32(ptr, 0);
    printf("Read back value at offset 0: 0x%08X\n", read_val0);

    // 驗證寫入是否正確
    if (read_val0 == new_val)
    {
        printf("Data verification successful!\n");
    }
    else
    {
        printf("Data verification failed!\n");
    }

    write32(ptr, 0, val);
    printf("Written 0x%08X to BAR0 offset 0.\n", val);
    printf("Read back value at offset 0: 0x%08X\n", read32(ptr, 0));
}
// static MyPCIData_T *GetMyPCIData(int bus, int device, int function);
// static MyPCIData_T *pMyPciData;

// input: bus, device, function
int DeviceSearch()
{
    std::vector<std::string> sysfs_path;
    pci_access *pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    char namebuf[1024], classbuf[1024];

    for (pci_dev *dev = pacc->devices; dev; dev = dev->next)
    {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES);
        if (dev->device_class == 0x0200)
        {
            printf("Network Device: %02x:%02x.%d\n", dev->bus, dev->dev, dev->func);
            printf("  Vendor ID: %04x\n", dev->vendor_id);
            printf("  Device ID: %04x\n", dev->device_id);
            printf("  Class: %04x\n", dev->device_class);
            printf("  Name: %s\n", pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id));
            printf("  Class Name: %s\n", pci_lookup_name(pacc, classbuf, sizeof(classbuf), PCI_LOOKUP_CLASS, dev->device_class));
            sysfs_path.push_back(get_sysfs_path(SYSFS_PCI_DEVICES, dev->domain, dev->bus, dev->dev, dev->func));
            printf("  Sysfs Path: %s\n", sysfs_path[sysfs_path.size() - 1].c_str());
            size_t length = sizeof(dev->base_addr) / sizeof(dev->base_addr[0]);
            for (int i = 0; i < length; i++)
            {
                printf("  BAR%d: 0x%08lx, Size: %u bytes\n", i, dev->base_addr[i], (uint32_t)dev->size[i]);
            }
            printf("\n");
        }
    }
    std::vector<void *> bar0;

    for (int i = 0; i < sysfs_path.size(); i++)
    {
        // mappingtest((char *)sysfs_path[i].c_str());
        // void *bar0=mapping((char *)sysfs_path[i].c_str());
        bar0.push_back(mapping((char *)sysfs_path[i].c_str()));
        readwriteTest(bar0[i]);
        // munmap(bar0, getpagesize()); // 解除映射
    }

    for (int i = 0; i < sysfs_path.size(); i++)
    {
        munmap(bar0[i], getpagesize()); // 解除映射
    }
    printf("\n");
    return 0;
}

// out
std::vector<int> scanPCI()
{
    return {0x1234, 0x5678, 0x9ABC};
}

int readRegister(int reg)
{
    std::cout << "Reading register: " << reg << std::endl;
    return reg + 10;
}

void writeRegister(int reg, int value)
{
    std::cout << "Writing " << value << " to register: " << reg << std::endl;
}
