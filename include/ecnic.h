#ifndef ECNIC_H
#define ECNIC_H

#include <vector>

std::vector<int> scanPCI();
int readRegister(int reg);
void writeRegister(int reg, int value);

#endif // ECNIC_H