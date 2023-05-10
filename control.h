#ifndef CONTROL_H
#define CONTROL_H

#include <sdios.h>
#include "constants.h"
#include "core.h"

void control_latchHighAddress();
void control_latchLowAddress();
void control_latchControl();
uint8_t control_setControlForBootloaderWrite(uint8_t controlStatus);
uint8_t control_setControlBit(uint8_t controlPin, uint8_t controlStatus);
uint8_t control_clearControlBit(uint8_t controlPin, uint8_t controlStatus);
void control_haltMSX();
void control_releaseMSX();
uint8_t control_deassertAddress16(uint8_t controlStatus);
uint8_t control_assertAddress16(uint8_t controlStatus);
uint8_t control_deassertRead(uint8_t controlStatus);
uint8_t control_assertWrite(uint8_t controlStatus);
uint8_t control_assertRead(uint8_t controlStatus);
void control_selectRAM();
void control_deselectRAM();
uint8_t control_clearReadAndWrite(uint8_t controlStatus);
uint8_t control_handover(uint8_t controlStatus);
uint8_t control_takeover(uint8_t controlStatus);
uint8_t control_setChipSelect(uint8_t cs, uint8_t controlStatus);

#endif