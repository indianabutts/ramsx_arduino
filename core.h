#ifndef CORE_H
#define CORE_H

#include <sdios.h>
#include "constants.h"
#include "control.h"
#include "sd_utilities.h"

void core_setDataPinsValue(byte data);
void core_initializeControlPins();
void core_initializeDataPinsForWrite();
void core_initializeDataPinsForRead();
uint8_t core_readDataPinsValue();
void core_writeDataToAddress(uint16_t address, uint8_t data);
uint8_t core_readDataFromAddress(uint8_t *data, uint8_t controlStatus, uint16_t address);
bool core_checkForCommandSignal();
uint8_t core_writeFileToSRAM(const file_t &romFile, uint8_t controlStatus);

#endif