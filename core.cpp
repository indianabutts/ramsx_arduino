#include "core.h"

void core_haltMSX(){
  PORTC &= B101111;
}

void core_releaseMSX(){
  PORTC |= B010000;
}

void core_latchHighAddress(){
  PORTC |= B000010;
  PORTC &= B111101;
}

void core_latchLowAddress(){
  PORTC |= B000001 ;
  PORTC &= B111110 ;
}
void core_latchControl(){
  PORTC |= B001000;
  PORTC &= B110111;
}

uint8_t core_deassertAddress16(uint8_t controlStatus){
  Serial.println("\n Deasserting A16");
  controlStatus = core_clearControlBit(CONTROL_A16, controlStatus);
  return controlStatus;
}

uint8_t core_assertAddress16(uint8_t controlStatus){
  Serial.println("\n Asserting A16");
  controlStatus = core_setControlBit(CONTROL_A16, controlStatus);
  return controlStatus;
}

void core_setDataPinsValue(byte data) {
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
}

uint8_t core_setControlForBootloaderWrite(uint8_t controlStatus){
  Serial.println("Setting up for Bootloader Write");
  controlStatus = core_clearControlBit(CONTROL_A16, controlStatus);
  controlStatus = core_clearControlBit(CONTROL_nWRITE, controlStatus);
  controlStatus = core_clearControlBit(CONTROL_COMM_RESET, controlStatus);
  controlStatus = core_setControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = core_setControlBit(CONTROL_HANDOVER, controlStatus);
  return controlStatus;
}

uint8_t core_setControlBit(uint8_t controlPin, uint8_t controlStatus){
  controlStatus |= (1<<controlPin);
  core_setDataPinsValue(controlStatus);
  core_latchControl();
  return controlStatus;
}
uint8_t core_clearControlBit(uint8_t controlPin, uint8_t controlStatus){
  controlStatus &= ~(1UL<<controlPin);
  core_setDataPinsValue(controlStatus);
  core_latchControl();
  return controlStatus;
}