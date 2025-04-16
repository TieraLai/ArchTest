#include "ecnic.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <unistd.h>

#define ERROR_BODY(message) printf("ERROR:\t%s:\t%s\n", __FUNCTION__, message);
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
    uint64_t new_val = val + 1;
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
    // close(fd);
    return ptr;
}

static void readwriteTest(void *ptr)
{
    printf("\nRead/Write Test.\n");
    // 寫入和讀取測試
    uint32_t val = read32(ptr, 0);
    uint32_t new_val = 0x08000241; // 0x001C0001;
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

#define MAX_PATH_LENGTH 256
void n2hexstr(unsigned int w, size_t hex_len, char *buffer)
{
    static const char *digits = "0123456789ABCDEF";
    for (size_t i = 0; i < hex_len; ++i)
    {
        buffer[hex_len - 1 - i] = digits[w & 0x0F];
        w >>= 4;
    }
    buffer[hex_len] = '\0'; // 給字串加上結束符
}
// 生成 sysfs 路徑
void get_sysfs_path(const char *sysfs_base, int domain, int bus, int dev, int func, char *path_buffer)
{
    // 轉換 domain, bus, dev, func 為十六進位字串
    char hex_domain[5], hex_bus[3], hex_dev[3], hex_func[2];
    n2hexstr(domain, 4, hex_domain);
    n2hexstr(bus, 2, hex_bus);
    n2hexstr(dev, 2, hex_dev);
    n2hexstr(func, 1, hex_func);

    // 組合成完整的路徑字串
    snprintf(path_buffer, MAX_PATH_LENGTH, "%s/%s:%s:%s.%s/resource0",
             sysfs_base, hex_domain, hex_bus, hex_dev, hex_func);

    // 直接將 domain, bus, dev, func 轉換為十六進位字串並組合成路徑
    snprintf(path_buffer, MAX_PATH_LENGTH, "%s/%04X:%02X:%02X.%X/resource0",
             sysfs_base, domain, bus, dev, func);
}

// For Debug purpose...
void PrintPciData(
    int Num, ULONG DevNum, // logical slot number for the PCI adapter
    ULONG FunNum,          // function number on the specified adapter
    ULONG bus,             // bus number
    PPCI_COMMON_CONFIG PciData)
{
    printf("\n--- No. %d PciData: -------------------------\n", Num);
    printf("BusNumber:\t\t%ld\n", bus);
    printf("DeviceNumber:\t\t%ld\n", DevNum);
    printf("FunctionNumber:\t\t%ld\n", FunNum);
    printf("VendorID:\t\t0x%x\n", PciData->VendorID);
    printf("DeviceID:\t\t0x%x\n", PciData->DeviceID);
    printf("Command:\t\t0x%x\n", PciData->Command);
    printf("Status:\t\t\t0x%x\n", PciData->Status);
    printf("RevisionID:\t\t0x%x\n", PciData->RevisionID);
    printf("ProgIf:\t\t\t0x%x\n", PciData->ProgIf);
    printf("SubClass:\t\t0x%x\n", PciData->SubClass);
    printf("BaseClass:\t\t0x%x\n", PciData->BaseClass);
    printf("CacheLineSize:\t\t0x%x\n", PciData->CacheLineSize);
    printf("LatencyTimer:\t\t0x%x\n", PciData->LatencyTimer);
    printf("HeaderType:\t\t0x%x\n", PciData->HeaderType);
    printf("BIST:\t\t\t0x%x\n", PciData->BIST);
    printf("BaseAddresses[0]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[0]);
    printf("BaseAddresses[1]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[1]);
    printf("BaseAddresses[2]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[2]);
    printf("BaseAddresses[3]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[3]);
    printf("BaseAddresses[4]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[4]);
    printf("BaseAddresses[5]:\t0x%08lx\n", PciData->u.type0.BaseAddresses[5]);
    printf("ROMBaseAddress:\t\t0x%08lx\n", PciData->u.type0.ROMBaseAddress);
    printf("InterruptLine:\t\t%d\n", PciData->u.type0.InterruptLine);
    printf("InterruptPin:\t\t%d\n", PciData->u.type0.InterruptPin);
    printf("MinimumGrant:\t\t%d\n", PciData->u.type0.MinimumGrant);
    printf("MaximumLatency:\t\t%d\n", PciData->u.type0.MaximumLatency);
}

ULONG RtGetBusDataByOffset_1(BUS_DATA_TYPE BusDataType, ULONG BusNumber, ULONG SlotNumber, PVOID pBuffer, ULONG Offset, ULONG Length)
{
    // only support PCIConfiguration
    if (BusDataType != PCIConfiguration)
        return 0;

    // init pBuffer
    PPCI_COMMON_CONFIG pPciInfo = (PPCI_COMMON_CONFIG)pBuffer;
    memset(pPciInfo, 0, sizeof(PCI_COMMON_CONFIG));
    PPCI_SLOT_NUMBER pSlotNumber = (PPCI_SLOT_NUMBER)&SlotNumber;
    ULONG DevNum = pSlotNumber->u.bits.DeviceNumber;
    ULONG FunNum = pSlotNumber->u.bits.FunctionNumber;

    // specify busnumber and slotnumber(DeviceNumber,FunctionNumber)
    struct pci_dev *dev = NULL;
    struct pci_access *pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    int isBusExist = 0;
    for (struct pci_dev *tmpdev = pacc->devices; tmpdev; tmpdev = tmpdev->next)
    {
        pci_fill_info(tmpdev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_ROM_BASE | PCI_FILL_CLASS);
        if (tmpdev->bus == BusNumber)
        {
            isBusExist = 1;
            if (tmpdev->dev == DevNum && tmpdev->func == FunNum)
            {
                dev = tmpdev;
                break;
            }
        }
    }

    if (isBusExist == 0)
    {
        pci_cleanup(pacc); // 釋放資源
        return 0;
    }

    if (dev == NULL && isBusExist == 1)
    {
        pPciInfo->VendorID = PCI_INVALID_VENDORID;
        pci_cleanup(pacc); // 釋放資源
        return 2;
    }

    // 填充主要的通用字段
    pPciInfo->VendorID = dev->vendor_id;
    pPciInfo->DeviceID = dev->device_id;
    pPciInfo->Command = pci_read_word(dev, PCI_COMMAND);
    pPciInfo->Status = pci_read_word(dev, PCI_STATUS);
    pPciInfo->RevisionID = pci_read_byte(dev, PCI_REVISION_ID);
    pPciInfo->ProgIf = pci_read_byte(dev, PCI_CLASS_PROG);
    pPciInfo->SubClass = dev->device_class & 0xFF; // 類別代碼的低位字節
    pPciInfo->BaseClass = dev->device_class >> 8;  // 類別代碼的高位字節
    pPciInfo->CacheLineSize = pci_read_byte(dev, PCI_CACHE_LINE_SIZE);
    pPciInfo->LatencyTimer = pci_read_byte(dev, PCI_LATENCY_TIMER);
    pPciInfo->HeaderType = pci_read_byte(dev, PCI_HEADER_TYPE);
    pPciInfo->BIST = pci_read_byte(dev, PCI_BIST);

    for (int i = 0; i < PCI_TYPE0_ADDRESSES; ++i)
    {
        pPciInfo->u.type0.BaseAddresses[i] = dev->base_addr[i];
    }
    pPciInfo->u.type0.ROMBaseAddress = pci_read_word(dev, PCI_ROM_ADDRESS);
    pPciInfo->u.type0.InterruptLine = pci_read_byte(dev, PCI_INTERRUPT_LINE);
    pPciInfo->u.type0.InterruptPin = pci_read_byte(dev, PCI_INTERRUPT_PIN);
    pPciInfo->u.type0.MinimumGrant = pci_read_byte(dev, PCI_MIN_GNT);
    pPciInfo->u.type0.MaximumLatency = pci_read_byte(dev, PCI_MAX_LAT);

    ULONG struct_size = sizeof(PCI_COMMON_CONFIG);
    ULONG bytes_written = (Offset < struct_size) ? (struct_size - Offset) : 0;
    // 確保不超過緩衝區長度
    bytes_written = (bytes_written > Length) ? Length : bytes_written;

    pci_cleanup(pacc); // 釋放資源
    return bytes_written;
}
ULONG RtGetBusDataByOffset(BUS_DATA_TYPE BusDataType, ULONG BusNumber, ULONG SlotNumber, PVOID pBuffer, ULONG Offset, ULONG Length)
{
    // only support PCIConfiguration
    if (BusDataType != PCIConfiguration)
        return 0;

    // init pBuffer
    PPCI_COMMON_CONFIG pPciInfo = (PPCI_COMMON_CONFIG)pBuffer;
    PPCI_SLOT_NUMBER pSlotNumber = (PPCI_SLOT_NUMBER)&SlotNumber;

    // specify busnumber and slotnumber(DeviceNumber,FunctionNumber)
    struct pci_access *pacc = pci_alloc();
    struct pci_dev *dev;
    pci_init(pacc);
    pci_scan_bus(pacc);

    // dev = pci_get_dev(pacc, 0, BusNumber, pSlotNumber->u.bits.DeviceNumber, pSlotNumber->u.bits.FunctionNumber);
    for (struct pci_dev *tmpdev = pacc->devices; tmpdev; tmpdev = tmpdev->next)
    {
        pci_fill_info(tmpdev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_ROM_BASE | PCI_FILL_CLASS);
        if (tmpdev->bus == BusNumber && tmpdev->dev == pSlotNumber->u.bits.DeviceNumber && tmpdev->func == pSlotNumber->u.bits.FunctionNumber)
        {
            dev = tmpdev;
            break;
        }
    }
    if (dev == NULL)
    {
        pPciInfo->VendorID = PCI_INVALID_VENDORID;
        pci_cleanup(pacc); // 釋放資源
        return 2;
    }

    pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_ROM_BASE | PCI_FILL_CLASS);
    // 填充主要的通用字段
    pPciInfo->VendorID = dev->vendor_id;
    pPciInfo->DeviceID = dev->device_id;
    pPciInfo->Command = pci_read_word(dev, PCI_COMMAND);
    pPciInfo->Status = pci_read_word(dev, PCI_STATUS);
    pPciInfo->RevisionID = pci_read_byte(dev, PCI_REVISION_ID);
    pPciInfo->ProgIf = pci_read_byte(dev, PCI_CLASS_PROG);
    pPciInfo->SubClass = dev->device_class & 0xFF; // 類別代碼的低位字節
    pPciInfo->BaseClass = dev->device_class >> 8;  // 類別代碼的高位字節
    pPciInfo->CacheLineSize = pci_read_byte(dev, PCI_CACHE_LINE_SIZE);
    pPciInfo->LatencyTimer = pci_read_byte(dev, PCI_LATENCY_TIMER);
    pPciInfo->HeaderType = pci_read_byte(dev, PCI_HEADER_TYPE);
    pPciInfo->BIST = pci_read_byte(dev, PCI_BIST);
    for (int i = 0; i < PCI_TYPE0_ADDRESSES; ++i)
    {
        pPciInfo->u.type0.BaseAddresses[i] = dev->base_addr[i];
    }
    pPciInfo->u.type0.ROMBaseAddress = pci_read_word(dev, PCI_ROM_ADDRESS);
    pPciInfo->u.type0.InterruptLine = pci_read_byte(dev, PCI_INTERRUPT_LINE);
    pPciInfo->u.type0.InterruptPin = pci_read_byte(dev, PCI_INTERRUPT_PIN);
    pPciInfo->u.type0.MinimumGrant = pci_read_byte(dev, PCI_MIN_GNT);
    pPciInfo->u.type0.MaximumLatency = pci_read_byte(dev, PCI_MAX_LAT);
    pPciInfo->domain = dev->domain;

    ULONG struct_size = sizeof(PCI_COMMON_CONFIG);
    ULONG bytes_written = (Offset < struct_size) ? (struct_size - Offset) : 0;
    // 確保不超過緩衝區長度
    bytes_written = (bytes_written > Length) ? Length : bytes_written;

    pci_cleanup(pacc); // 釋放資源
    return bytes_written;
}
ULONG RtSetBusDataByOffset(BUS_DATA_TYPE BusDataType, ULONG BusNumber, ULONG SlotNumber, PVOID pBuffer, ULONG Offset, ULONG Length)
{
    // only support PCIConfiguration
    if (BusDataType != PCIConfiguration)
        return 0;

    // init pBuffer
    PPCI_COMMON_CONFIG pPciInfo = (PPCI_COMMON_CONFIG)pBuffer;
    PPCI_SLOT_NUMBER pSlotNumber = (PPCI_SLOT_NUMBER)&SlotNumber;

    // specify busnumber and slotnumber(DeviceNumber,FunctionNumber)
    struct pci_access *pacc = pci_alloc();
    struct pci_dev *dev;
    pci_init(pacc);
    pci_scan_bus(pacc);

    dev = pci_get_dev(pacc, pPciInfo->domain, BusNumber, pSlotNumber->u.bits.DeviceNumber, pSlotNumber->u.bits.FunctionNumber);
    if (dev == NULL)
    {
        pci_free_dev(dev);
        pci_cleanup(pacc); // 釋放資源
        printf("\ndev == NULL\n");
        return 0;
    }

    pci_fill_info(dev, PCI_FILL_IDENT);
    // wire the command register
    int ret = pci_write_word(dev, PCI_COMMAND, pPciInfo->Command);
    if (ret != 1)
    {
        // Failed to set Command Register
        pci_free_dev(dev);
        pci_cleanup(pacc); // 釋放資源
        printf("\nret!=0\n");
        return 0;
    }
    printf("ret:\t\t%d\n", ret);
    ULONG struct_size = sizeof(PCI_COMMON_CONFIG);
    ULONG bytes_written = (Offset < struct_size) ? (struct_size - Offset) : 0;
    // 確保不超過緩衝區長度
    bytes_written = (bytes_written > Length) ? Length : bytes_written;

    pci_free_dev(dev);
    pci_cleanup(pacc); // 釋放資源

    return bytes_written;
}

PVOID RtMapMemory(ULONG BusNumber, ULONG SlotNumber, PVOID pBuffer)
{
    char resource_path[MAX_PATH_LENGTH];
    // init pBuffer
    PPCI_COMMON_CONFIG pPciInfo = (PPCI_COMMON_CONFIG)pBuffer;
    PPCI_SLOT_NUMBER pSlotNumber = (PPCI_SLOT_NUMBER)&SlotNumber;

    snprintf(resource_path, MAX_PATH_LENGTH, "%s/%04lx:%02lx:%02lx.%lx/resource0",
             "/sys/bus/pci/devices", pPciInfo->domain, BusNumber, pSlotNumber->u.bits.DeviceNumber, pSlotNumber->u.bits.FunctionNumber);
    printf("resource_path:\t%s\n", resource_path);
    int fd;
    if ((fd = open(resource_path, O_RDWR | O_SYNC)) == -1)
    {
        printf("open failed\n");
        fflush(stdout);
        return NULL;
    }

    unsigned int map_size;
    struct stat st;
    stat(resource_path, &st); // get BAR size through sysfs file size
    map_size = st.st_size;
    void *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // 131072 map size = 128KB (131072 bytes) = 32 * 4096 bytes
    if (ptr == MAP_FAILED)
    {
        printf("mmap failed\n");
        fflush(stdout);
        close(fd);
        return NULL;
    }
    close(fd);
    printf("PCI Memory mapped to user space at address %p.\n", ptr);
    printf("PCI BAR0 = 0x%08X\n", *((unsigned int *)ptr));

    return ptr;
}
#define PAGE_PRESENT (1ULL << 63)
#define PAGE_PFN_MASK ((1ULL << 55) - 1)
unsigned long get_physical_address(void *virtual_address)
{
    int pagemap_fd;
    unsigned long virtual_addr = (unsigned long)virtual_address;
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long page_offset = (virtual_addr / page_size) * sizeof(uint64_t);
    uint64_t pfn_entry;

    // 打开 pagemap 文件
    pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0)
    {
        perror("Failed to open /proc/self/pagemap");
        return -1;
    }

    // 读取对应的 pagemap 条目
    if (pread(pagemap_fd, &pfn_entry, sizeof(uint64_t), page_offset) != sizeof(uint64_t))
    {
        perror("Failed to read pagemap entry");
        close(pagemap_fd);
        return -1;
    }

    // 检查页面是否存在
    if (!(pfn_entry & PAGE_PRESENT))
    {
        fprintf(stderr, "Page not present in memory\n");
        close(pagemap_fd);
        return -1;
    }

    // 提取物理页帧号并计算物理地址
    unsigned long pfn = pfn_entry & PAGE_PFN_MASK;
    unsigned long phys_addr = (pfn * page_size) + (virtual_addr % page_size);

    close(pagemap_fd);
    return phys_addr;
}
// input: bus, device, function
int DeviceSearch_printAll()
{
    // std::vector<std::string> sysfs_path;
    pci_access *pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    char namebuf[1024], classbuf[1024];
    char sysfs_path[PCI_MAX_DEVICES][MAX_PATH_LENGTH];
    int index = 0;

    for (pci_dev *dev = pacc->devices; dev; dev = dev->next)
    {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES);
        if (dev->device_class == 0x0200)
        {
            printf("Network Device: %02x:%02x.%d\n", dev->bus, dev->dev, dev->func);
            printf("  Domain ID: %04x\n", dev->domain);
            printf("  Vendor ID: %04x\n", dev->vendor_id);
            printf("  Device ID: %04x\n", dev->device_id);
            printf("  Class: %04x\n", dev->device_class);
            printf("  Name: %s\n", pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id));
            printf("  Class Name: %s\n", pci_lookup_name(pacc, classbuf, sizeof(classbuf), PCI_LOOKUP_CLASS, dev->device_class));
            // sysfs_path.push_back(get_sysfs_path(SYSFS_PCI_DEVICES, dev->domain, dev->bus, dev->dev, dev->func));
            // printf("  Sysfs Path: %s\n", sysfs_path[sysfs_path.size() - 1].c_str());
            get_sysfs_path(SYSFS_PCI_DEVICES, dev->domain, dev->bus, dev->dev, dev->func, sysfs_path[index]);
            printf("  Sysfs Path: %s\n", sysfs_path[index]);
            size_t length = sizeof(dev->base_addr) / sizeof(dev->base_addr[0]);
            for (int i = 0; i < length; i++)
            {
                printf("  BAR%d: 0x%08lx, Size: %u bytes\n", i, dev->base_addr[i], (uint32_t)dev->size[i]);
            }
            printf("\n");
            index++;
        }
    }

    // std::vector<void *> bar0;
    void *bar0[PCI_MAX_DEVICES];
    for (int i = 0; i < index; i++)
    {
        printf("  Sysfs Path: %s\n", sysfs_path[i]);
    }

    for (int i = 0; i < index; i++)
    {
        // mappingtest((char *)sysfs_path[i].c_str());
        // void *bar0=mapping((char *)sysfs_path[i].c_str());
        bar0[i] = mapping((char *)sysfs_path[i]);
        // readwriteTest(bar0[i]);
        // munmap(bar0, getpagesize()); // 解除映射
    }

    unsigned long phys_addr[PCI_MAX_DEVICES];
    for (int i = 0; i < index; i++)
    {
        phys_addr[i] = get_physical_address(bar0[i]);
        printf("Physical address: 0x%lx\n", phys_addr[i]);

        if (phys_addr[i] != (unsigned long)-1)
        {
            printf("Physical address: 0x%lx\n", phys_addr[i]);
        }
    }

    for (int i = 0; i < index; i++)
    {
        munmap(bar0[i], getpagesize()); // 解除映射
    }
    printf("\n");
    return 0;
}
BOOL RtUnmapMemory(PVOID pVirtualAddress)
{
    return munmap(pVirtualAddress, getpagesize()) == 0 ? TRUE : FALSE;
}
#define PCI_DISABLE_INTERRUPT (0x0400)
#define TX_DESC_RING_SIZE 32 // 发送描述符环的大小

// 描述符结构体
struct tx_desc
{
    uint64_t addr;    // 数据包的物理地址
    uint16_t length;  // 数据包长度
    uint16_t cmd;     // 命令和状态
    uint8_t status;   // 状态
    uint8_t reserved; // 保留字段
};

// 发送描述符环
struct tx_desc tx_desc_ring[TX_DESC_RING_SIZE];

// 初始化发送描述符
void init_tx_desc_ring(void *bar0)
{
    volatile uint32_t *regs = (volatile uint32_t *)bar0;
    uint64_t desc_base = (uint64_t)&tx_desc_ring;

    // 设置发送描述符环的物理地址
    regs[0x0380 / 4] = (uint32_t)(desc_base & 0xFFFFFFFF);         // TXDCTL (低32位)
    regs[0x0384 / 4] = (uint32_t)((desc_base >> 32) & 0xFFFFFFFF); // TXDCTL (高32位)

    // 清空描述符环
    memset(tx_desc_ring, 0, sizeof(tx_desc_ring));

    printf("TX Descriptor ring initialized.\n");
}

// 发送数据
void send_packet(void *bar0, uint8_t *data, uint16_t length)
{
    volatile uint32_t *regs = (volatile uint32_t *)bar0;

    // 获取当前可用的描述符
    struct tx_desc *desc = &tx_desc_ring[0];

    // 填充描述符
    desc->addr = (uint64_t)data; // 数据包的地址
    desc->length = length;       // 数据包长度
    desc->cmd = 0x1;             // 设置发送命令（例如启动发送）

    // 启动发送操作
    regs[0x0400 / 4] = 0x1; // TXCTRL 启动发送（示例，具体寄存器配置依赖硬件文档）

    printf("Packet sent with length: %d\n", length);
}
int check_tx_status(int idx)
{
    struct tx_desc *desc = &tx_desc_ring[idx];

    // 检查描述符的状态
    if (desc->status & 0x1)
    { // 检查 DD 位
        printf("Descriptor %d: Packet sent successfully.\n", idx);
        return 1;
    }
    else
    {
        printf("Descriptor %d: Sending in progress.\n", idx);
        return 0;
    }
}
#define TDT 0x3818 // Transmit Descriptor Tail
#define TDH 0x3810 // Transmit Descriptor Head
void check_tx_ring_status(void *bar0)
{
    volatile uint32_t *regs = (volatile uint32_t *)bar0;
    uint32_t tdh = regs[TDH / 4]; // 读取发送队列的头指针
    uint32_t tdt = regs[TDT / 4]; // 读取发送队列的尾指针

    printf("Transmit Descriptor Head (TDH): %u\n", tdh);
    printf("Transmit Descriptor Tail (TDT): %u\n", tdt);

    if (tdh == tdt)
    {
        printf("All descriptors processed. Transmit queue is empty.\n");
    }
    else
    {
        printf("Transmit queue still in progress.\n");
    }
}

// OS.h
#define LENGTH (2 * 1024 * 1024)
#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* arch specific */
#endif
PVOID RtAllocateContiguousMemory(ULONG Length, LARGE_INTEGER PhysicalAddress)
{
    void *addr;

    // 使用 mmap 分配 HugePages 內存
    addr = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE,
                MAP_HUGETLB | MAP_ANONYMOUS| MAP_PRIVATE, -1, 0);

    if (addr == MAP_FAILED)
    {
        perror("mmap failed");
        return NULL;
    }

    return addr;
}

BOOL RtFreeContiguousMemory(PVOID pVirtualAddress)
{
    return munmap(pVirtualAddress, LENGTH) == 0 ? TRUE : FALSE;
}

#define PAGE_SIZE 4096
LARGE_INTEGER RtGetPhysicalAddress(PVOID pVirtualAddress)
{
    LARGE_INTEGER result;
    result.QuadPart = -1; // or any appropriate value

    unsigned long offset = (unsigned long)pVirtualAddress / PAGE_SIZE * sizeof(unsigned long);
    unsigned long page_frame;

    // 打开 /proc/self/pagemap 文件
    int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd == -1)
    {
        perror("Failed to open /proc/self/pagemap");
        return result;
    }

    // 读取虚拟地址所在页面的信息
    if (pread(pagemap_fd, &page_frame, sizeof(page_frame), offset) != sizeof(page_frame))
    {
        perror("Failed to read from pagemap");
        close(pagemap_fd);
        return result;
    }

    // 关闭 pagemap 文件
    close(pagemap_fd);

    // 判断页是否有效
    if (!(page_frame & (1ULL << 63)))
    {
        fprintf(stderr, "Page is not present in physical memory.\n");
        return result;
    }

    // 获取物理页帧地址
    unsigned long phy_addr = (page_frame & ((1ULL << 55) - 1)) * PAGE_SIZE + ((unsigned long)pVirtualAddress % PAGE_SIZE);
    result.QuadPart = (LONGLONG)phy_addr;
    return result;
}

static void check_bytes(char *addr)
{
    printf("First hex is %x\n", *((unsigned int *)addr));
}

static void write_bytes(char *addr)
{
    unsigned long i;

    for (i = 0; i < LENGTH; i++)
        *(addr + i) = (char)i;
}

static void read_bytes(char *addr)
{
    unsigned long i;

    check_bytes(addr);
    for (i = 0; i < LENGTH; i++)
        if (*(addr + i) != (char)i)
        {
            printf("Mismatch at %lu\n", i);
            break;
        }
}

//   // readwriteTest(pCsrBase[0]);
//     volatile uint32_t *regs = (volatile uint32_t *)pCsrBase;

//     // Intel 82574 寄存器偏移
// #define CTRL 0x0000   // 控制寄存器
// #define STATUS 0x0008 // 状态寄存器
// #define RXCTRL 0x0100 // 接收控制寄存器
// #define TXCTRL 0x0400 // 发送控制寄存器
//     // 1. 软复位网卡
//     regs[CTRL / 4] |= (1 << 26); // 设置 RST 位
//     usleep(10000);               // 等待复位完成

//     // 2. 检查复位完成
//     if (regs[CTRL / 4] & (1 << 26))
//     {
//         printf("Reset failed.\n");
//     }
//     else
//     {
//         printf("Reset completed successfully.\n");
//     }

//     // 3. 启用网卡的接收和发送功能
//     regs[CTRL / 4] |= (1 << 5) | (1 << 6); // 设置 ASDE 和 SLU 位
//     regs[RXCTRL / 4] = 0x1;                // 启用接收功能
//     regs[TXCTRL / 4] = 0x1;                // 启用发送功能

//     printf("NIC initialized successfully.\n");

//     // 初始化发送描述符环
//     init_tx_desc_ring(pCsrBase);

//     // 发送一个简单的示例数据包
//     uint8_t packet[] = "Hello, Network!";
//     send_packet(pCsrBase, packet, sizeof(packet) - 1);

// // 等待发送完成
//     // int idx = 0;
//     while (1) {
//         if(check_tx_status(0))
//         {
//             break;
//         }
//         usleep(1000); // 每 1 毫秒检查一次状态
//     }
//  // 检查发送队列状态
//     // check_tx_ring_status(pCsrBase);

int DeviceSearch_test()
{
    MyPCIData_T pMyPciData;
    PCI_COMMON_CONFIG PciData; // 不需要指針
    ULONG bytesWritten;

    // 初始化設備位置
    pMyPciData.busNumber = 1;
    pMyPciData.slotNumber.u.bits.Reserved = 0;
    pMyPciData.slotNumber.u.bits.DeviceNumber = 0;
    pMyPciData.slotNumber.u.bits.FunctionNumber = 0;

    // todo : implement the function RtGetBusDataByOffset
    bytesWritten = RtGetBusDataByOffset(
        PCIConfiguration,                // type of bus data to be retrieved
        pMyPciData.busNumber,            // zero-based number of the bus
        pMyPciData.slotNumber.u.AsULONG, // logical slot number
        &PciData,                        // pointer to a buffer for configuration information
        0,                               // byte offset into buffer
        PCI_COMMON_HDR_LENGTH            // length of buffer
    );

    if (bytesWritten == 0 || (bytesWritten == 2 && PciData.VendorID == PCI_INVALID_VENDORID))
    {
        // out of PCI buses done
        return 0; // found no device matched
    }

    PrintPciData(0, pMyPciData.slotNumber.u.bits.DeviceNumber, pMyPciData.slotNumber.u.bits.FunctionNumber, pMyPciData.busNumber, &PciData);

    // test write
    printf("\ntest write\n");
    USHORT command_old = 0x17;
    USHORT command_new = (PCI_ENABLE_MEMORY_SPACE);
    printf("command_old:\t\t0x%x\n", command_old);
    printf("command_new:\t\t0x%x\n", command_new);

    PciData.Command = command_new;
    bytesWritten = RtSetBusDataByOffset(
        PCIConfiguration,                // type of bus data to be retrieved
        pMyPciData.busNumber,            // zero-based number of the bus
        pMyPciData.slotNumber.u.AsULONG, // logical slot number
        &PciData,                        // pointer to a buffer for configuration information
        0,                               // byte offset into buffer
        PCI_COMMON_HDR_LENGTH            // length of buffer
    );
    if (bytesWritten == 0)
    {

        printf("\ntest write error\n");
        return 0; // found no device matched
    }

    // todo : implement the function RtGetBusDataByOffset
    bytesWritten = RtGetBusDataByOffset(
        PCIConfiguration,                // type of bus data to be retrieved
        pMyPciData.busNumber,            // zero-based number of the bus
        pMyPciData.slotNumber.u.AsULONG, // logical slot number
        &PciData,                        // pointer to a buffer for configuration information
        0,                               // byte offset into buffer
        PCI_COMMON_HDR_LENGTH            // length of buffer
    );

    if (bytesWritten == 0 || (bytesWritten == 2 && PciData.VendorID == PCI_INVALID_VENDORID))
    {
        // out of PCI buses done
        return 0; // found no device matched
    }

    PrintPciData(0, pMyPciData.slotNumber.u.bits.DeviceNumber, pMyPciData.slotNumber.u.bits.FunctionNumber, pMyPciData.busNumber, &PciData);

    // restore the original value
    printf("\ntest restore\n");
    PciData.Command = command_old;
    bytesWritten = RtSetBusDataByOffset(
        PCIConfiguration,                // type of bus data to be retrieved
        pMyPciData.busNumber,            // zero-based number of the bus
        pMyPciData.slotNumber.u.AsULONG, // logical slot number
        &PciData,                        // pointer to a buffer for configuration information
        0,                               // byte offset into buffer
        PCI_COMMON_HDR_LENGTH            // length of buffer
    );
    if (bytesWritten == 0)
    {
        printf("\ntest write error\n");
        return 0; // found no device matched
    }

    // todo : implement the function RtGetBusDataByOffset
    bytesWritten = RtGetBusDataByOffset(
        PCIConfiguration,                // type of bus data to be retrieved
        pMyPciData.busNumber,            // zero-based number of the bus
        pMyPciData.slotNumber.u.AsULONG, // logical slot number
        &PciData,                        // pointer to a buffer for configuration information
        0,                               // byte offset into buffer
        PCI_COMMON_HDR_LENGTH            // length of buffer
    );

    if (bytesWritten == 0 || (bytesWritten == 2 && PciData.VendorID == PCI_INVALID_VENDORID))
    {
        // out of PCI buses done
        return 0; // found no device matched
    }

    PrintPciData(0, pMyPciData.slotNumber.u.bits.DeviceNumber, pMyPciData.slotNumber.u.bits.FunctionNumber, pMyPciData.busNumber, &PciData);

    void *pCsrBase = RtMapMemory(pMyPciData.busNumber, pMyPciData.slotNumber.u.AsULONG, &PciData);
    if (pCsrBase == NULL)
    {
        printf("error BAR0 not a memory space.\n");
        return 0;
    }

    // U32_T NumTxDesc = 32;
    // U32_T NumRxDesc = 16;
    // U32_T BufferSize = 2048; // Default 2048 Byte
    // U32_T dwBlock;
    // // Allocate memory for packet buffers
    // dwBlock = NumTxDesc + NumRxDesc;
    // dwBlock *= BufferSize;

    // PHYSICAL_ADDRESS physaddrHighest;
    // physaddrHighest.QuadPart = 0xFFFFFFFF;
    // U8_T *pbyBlockFragmented = (U8_T *)RtAllocateContiguousMemory(dwBlock, physaddrHighest); // RtAllocateContiguousMemory is not available via RtWinApi

    // printf("Returned address is %p\n", pbyBlockFragmented);
    // check_bytes((char *)pbyBlockFragmented);
    // write_bytes((char *)pbyBlockFragmented);
    // read_bytes((char *)pbyBlockFragmented);
    // PHYSICAL_ADDRESS physaddrTo;
	// physaddrTo = RtGetPhysicalAddress(pbyBlockFragmented); // RtGetPhysicalAddress is not available via RtWinApi
    // printf("Returned address is = 0x%08lx\n", physaddrTo.LowPart);
    // printf("Returned address is = 0x%08lx\n", physaddrTo.HighPart);


    // RtFreeContiguousMemory(pbyBlockFragmented);
    RtUnmapMemory(pCsrBase);

    return 1;
}

int DeviceSearch()
{
    //DeviceSearch_printAll();
    DeviceSearch_test();
    return 1;
}

// out
// std::vector<int> scanPCI()
// {
//     return {0x1234, 0x5678, 0x9ABC};
// }
int *scanPCI()
{
    // 静态数组模拟 std::vector 行为
    static int pci_devices[3] = {0x1234, 0x5678, 0x9ABC};

    return pci_devices;
}

int readRegister(int reg)
{
    printf("Reading register:%d\n", reg);
    return reg + 10;
}

void writeRegister(int reg, int value)
{
    printf("Writing %d to register: %d \n", value, reg);
}
