#ifndef ECNIC_H
#define ECNIC_H
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h> //提供內存管理、程序控制和其他工具函數（例如 malloc()、exit() 等)
#include <vector>
#include "pci_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <pci/pci.h>   // 提供 PCI 設備的操作和信息檢索功能
#include <pci/types.h> // 提供 PCI 設備的操作和信息檢索功能
#include <sys/mman.h>  // 提供內存映射功能（例如 mmap()、munmap() 等）

#ifdef __cplusplus
}
#endif

std::vector<int> scanPCI();
int readRegister(int reg);
void writeRegister(int reg, int value);
int DeviceSearch();

typedef struct
{
    PCI_SLOT_NUMBER slotNumber;
    PCI_COMMON_CONFIG pciData;
    int busNumber;
} MyPCIData_T;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;



#endif // ECNIC_H