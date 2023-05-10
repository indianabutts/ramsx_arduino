#include "control.h"

void control_haltMSX(){
  PORTC &= B101111;
}

void control_releaseMSX(){
  PORTC |= B010000;
}

void control_latchHighAddress(){
  PORTC |= B000010;
  PORTC &= B111101;
}

void control_latchLowAddress(){
  PORTC |= B000001 ;
  PORTC &= B111110 ;
}
void control_latchControl(){
  PORTC |= B001000;
  PORTC &= B110111;
}


uint8_t control_deassertAddress16(uint8_t controlStatus){
  // Serial.println("\n Deasserting A16");
  controlStatus = control_clearControlBit(CONTROL_A16, controlStatus);
  return controlStatus;
}

uint8_t control_assertAddress16(uint8_t controlStatus){
  // Serial.println("\n Asserting A16");
  controlStatus = control_setControlBit(CONTROL_A16, controlStatus);
  return controlStatus;
}

void control_setControlForBootloaderWrite(uint8_t *controlStatus){
  // Serial.println("Setting up for Bootloader Write");
  (*controlStatus) = control_clearControlBit(CONTROL_A16, *controlStatus);
  (*controlStatus) = control_clearControlBit(CONTROL_nWRITE, *controlStatus);
  (*controlStatus) = control_clearControlBit(CONTROL_COMM_RESET, *controlStatus);
  (*controlStatus) = control_setControlBit(CONTROL_nREAD, *controlStatus);
  (*controlStatus) = control_setControlBit(CONTROL_HANDOVER, *controlStatus);
  return;
}

uint8_t control_setControlBit(uint8_t controlPin, uint8_t controlStatus){
  controlStatus |= (1<<controlPin);
  core_setDataPinsValue(controlStatus);
  control_latchControl();
  return controlStatus;
}
uint8_t control_clearControlBit(uint8_t controlPin, uint8_t controlStatus){
  controlStatus &= ~(1UL<<controlPin);
  core_setDataPinsValue(controlStatus);
  control_latchControl();
  return controlStatus;
}

uint8_t control_assertWrite(uint8_t controlStatus) {
  // Serial.println("\nAsserting Write");
  controlStatus = control_setControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = control_clearControlBit(CONTROL_nWRITE, controlStatus);
  return controlStatus;
}

uint8_t control_deassertRead(uint8_t controlStatus){
  // Serial.println("\nDeasserting Read");
  controlStatus = control_setControlBit(CONTROL_nREAD, controlStatus);
  return controlStatus;
}

uint8_t control_assertRead(uint8_t controlStatus) {
  // Serial.println("\nAsserting Read");
  controlStatus = control_clearControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = control_setControlBit(CONTROL_nWRITE, controlStatus);
  return controlStatus;
}

void control_selectRAM() {
  PORTC &= B111011;
}

void control_deselectRAM() {
  PORTC |= B000100;
}

uint8_t control_clearReadAndWrite(uint8_t controlStatus){
  // Serial.println("\nClearing Read and Write");
  controlStatus = control_setControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = control_setControlBit(CONTROL_nWRITE, controlStatus);
  return controlStatus;
}

uint8_t control_handover(uint8_t controlStatus) {
  Serial.println("\nHanding Over");
  controlStatus = control_clearControlBit(CONTROL_HANDOVER, controlStatus);
  return controlStatus;
}

uint8_t control_takeover(uint8_t controlStatus) {
  Serial.println("\nHanding Over");
  controlStatus = control_setControlBit(CONTROL_HANDOVER, controlStatus);
  return controlStatus;
}

uint8_t control_setChipSelect(uint8_t cs, uint8_t controlStatus){
  if(cs == 0 ){
    controlStatus = control_clearControlBit(CONTROL_SEL_A, controlStatus);
    controlStatus = control_clearControlBit(CONTROL_SEL_B, controlStatus);
  }
  if(cs & 1 == 1){
    controlStatus = control_setControlBit(CONTROL_SEL_A, controlStatus);
  }
  if(cs & 2 == 2 ){
    controlStatus = control_setControlBit(CONTROL_SEL_B, controlStatus);
  }
  return controlStatus;
}

