#ifndef ECNIC_H
#define ECNIC_H
#include <stdint.h>
#include <stddef.h>
#include "pci_def.h"
#include <vector>

std::vector<int> scanPCI();
int readRegister(int reg);
void writeRegister(int reg, int value);


typedef struct
{
    PCI_SLOT_NUMBER slotNumber;
    PCI_COMMON_CONFIG pciData;
    int busNumber;
} MyPCIData_T;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

int DeviceSearch(MyPCIData_T *pMyPciData);


#endif // ECNIC_H