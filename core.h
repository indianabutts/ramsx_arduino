#ifndef CORE_H
#define CORE_H

#include <sdios.h>
#include "constants.h"

void core_latchHighAddress();
void core_latchLowAddress();
void core_latchControl();
uint8_t core_setControlForBootloaderWrite(uint8_t controlStatus);
uint8_t core_setControlBit(uint8_t controlPin, uint8_t controlStatus);
uint8_t core_clearControlBit(uint8_t controlPin, uint8_t controlStatus);
void core_setDataPinsValue(byte data);
void core_haltMSX();
void core_releaseMSX();
uint8_t core_deassertAddress16(uint8_t controlStatus);
uint8_t core_assertAddress16(uint8_t controlStatus);


#endif