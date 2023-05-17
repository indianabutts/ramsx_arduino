#include "core.h"

void core_setDataPinsValue(byte data) {
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
}
uint8_t core_readDataPinsValue(){
  byte data = (PIND & B11111100)>>2;
  return data | (((PINB & B00000011))<<6);
}

uint8_t core_readDataFromAddress(uint8_t *data, uint8_t controlStatus, uint16_t address){
  core_initializeDataPinsForWrite();
  controlStatus = control_deassertRead(controlStatus);
  
  __asm__("nop\n\t");
  __asm__("nop\n\t"); 
  __asm__("nop\n\t");
  __asm__("nop\n\t"); 
  __asm__("nop\n\t");
  __asm__("nop\n\t"); 
  byte lowReadAddress = address & 0xFF;
  byte highReadAddress = (address >> 8) & 0xFF;
  core_setDataPinsValue(lowReadAddress);
  control_latchLowAddress();
  core_setDataPinsValue(highReadAddress);
  control_latchHighAddress();
  
  controlStatus = control_assertRead(controlStatus);
  core_initializeDataPinsForRead();
  control_selectRAM();
  __asm__("nop\n\t");
  __asm__("nop\n\t"); 
  __asm__("nop\n\t");
  __asm__("nop\n\t"); 
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  (*data) = core_readDataPinsValue(); 
  control_deselectRAM();
  return controlStatus;
}
bool core_checkForCommandSignal(){
  return !(PINC & B100000);
}

void core_writeDataToAddress(uint16_t address, uint8_t data){
  byte lowAddress = address & 0xFF;
  byte highAddress = (address >>8) & 0xFF;
  core_setDataPinsValue(lowAddress);
  control_latchLowAddress();
  core_setDataPinsValue(highAddress);
  control_latchHighAddress();
  core_setDataPinsValue(data);
  control_selectRAM();
  control_deselectRAM();
}

void core_initializeControlPins(){
  DDRC = B011111;
}

void core_initializeDataPinsForWrite() {
  DDRD = DDRD | B11111100;
  DDRB = DDRB | B00000011;
}
void core_initializeDataPinsForRead() {
  DDRD = DDRD & B00000011;
  DDRB = DDRB & B11111100;
}


uint8_t core_writeFileToSRAM(const file_t &romFile, uint8_t controlStatus){
  uint32_t romSize = (uint32_t) romFile.fileSize();

  romFile.seek(3);

  unsigned int offset = 0;
  uint8_t chipSelect = 0;
  byte peekValue = romFile.peek();

  if (peekValue >= 0x80) {
    offset = 0x8000;
    chipSelect = 1;
  } else if (peekValue >= 0x40 && peekValue < 0x80) {
    offset = 0x4000;
    chipSelect = 0;
  }

  if (romSize >= 32000) {
    offset = 0;
    chipSelect = 3;
  }

  if(romSize >=64000){
    offset = 0;
    chipSelect = 3;
  }
  controlStatus = control_setChipSelect(chipSelect, controlStatus);
  core_initializeDataPinsForWrite();
  Serial.print(F("\nStarting ROM Read with Offset "));
  Serial.print(offset, HEX);
  Serial.print(F(" and size "));
  if(romSize>1000){
  Serial.print(romSize/1000);
  Serial.print(F(" kbytes"));
  } else {
    Serial.print(romSize);
  Serial.print(F(" bytes"));
  }
  long int startTime = millis();
  unsigned int address = 0;
  controlStatus = control_assertWrite(controlStatus);
  romFile.seek(0);
  uint8_t data;

  core_writeDataToAddress(0x4000, 0xFF);
  core_writeDataToAddress(0x8000, 0xFF);
  core_writeDataToAddress(0x0000, 0xFF);
  while (romFile.available()) {
    unsigned int offsetAddress = address + offset;
    data = romFile.read();
    core_writeDataToAddress(offsetAddress, data);
    address++;
  }
  long int completeTime = millis();
  Serial.print(F("\nCompleted ROM Read in "));
  Serial.print(completeTime - startTime);
  Serial.print(F(" ms"));
  Serial.print(F("\nHanding over to MSX, using signal "));Serial.print(chipSelect);Serial.print(F("\n"));
  return controlStatus;
}