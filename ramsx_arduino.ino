#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <RingBuf.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

#include "constants.h"
#include "sd_utilities.h"
#include "core.h"

char* testName = "YASSIR ALREFAIE.     ";
uint8_t controlStatus = 0;

void setup() {

  
  setupControlPins();
  core_haltMSX();
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  
  setupDataPinsForWrite();
  
  controlStatus = core_setControlForBootloaderWrite(controlStatus);

  
  sd_t sd = sd_initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  file_t romFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }

  sd_displayDirectoryContent(sd, root, 0);
  while(true);
  // char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // char* filename = "testrom_write.rom";
  if(!romFile.open(&root, "out.rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  writeFileToSRAM(romFile); 
  handover();
  core_releaseMSX();
  clearReadOrWrite();
  while (true);

}

void loop() {
}

// void haltMSX(){
//   PORTC &= B101111;
// }

// void releaseMSX(){
//   PORTC |= B010000;
// }

// void deassertAddress16(){
//   Serial.println("\n Deasserting A16");
//   controlStatus = core_clearControlBit(CONTROL_A16, controlStatus);
// }

// void assertAddress16(){
//   Serial.println("\n Asserting A16");
//   controlStatus = core_setControlBit(CONTROL_A16, controlStatus);
// }


void setupControlPins(){
  DDRC = DDRC | B011111;
}
void setupDataPinsForWrite() {
  DDRD = DDRD | B11111100;
  DDRB = DDRB | B00000011;
  
}
void setupDataPinsForRead() {
  DDRD = DDRD & B00000011;
  DDRB = DDRB & B11111100;
}


// void core_setControlBit(uint8_t controlPin){
//   // Serial.print("Setting Control Bit "); Serial.print(controlPin);Serial.print("\r\n");
//   // Serial.print("Previous Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
//   controlStatus |= (1<<controlPin);
//   setDataPinsValue(controlStatus);
//   core_latchControl();
//   // Serial.print("New Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
// }
// void core_clearControlBit(uint8_t controlPin){
//   // Serial.print("Clearing Control Bit "); Serial.print(controlPin);Serial.print("\r\n");
//   // Serial.print("Previous Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
//   controlStatus &= ~(1UL<<controlPin);
//   setDataPinsValue(controlStatus);
//   core_latchControl();
//   // Serial.print("New Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
// }

void handover() {
  Serial.println("\nHanding Over");
  controlStatus = core_clearControlBit(CONTROL_HANDOVER, controlStatus);
}

void selectRAM() {
  PORTC &= B111011;
}

void deselectRAM() {
  PORTC |= B000100;
}

void assertWrite() {
  Serial.println("\nAsserting Write");
  controlStatus = core_setControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = core_clearControlBit(CONTROL_nWRITE, controlStatus);
}

void assertRead() {
  Serial.println("\nAsserting Read");
  controlStatus = core_clearControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = core_setControlBit(CONTROL_nWRITE, controlStatus);
}

uint8_t readData(){
  byte data = (PIND & B11111100)>>2;
  return data | (((PINB & B00000011))<<6);
}

void clearReadOrWrite(){
  Serial.println("\nClearing Read and Write");
  controlStatus = core_setControlBit(CONTROL_nREAD, controlStatus);
  controlStatus = core_setControlBit(CONTROL_nWRITE, controlStatus);
}


uint8_t readDataFromAddress(uint16_t address){
  deselectRAM();
  //deassertReadandWrite();
  setupDataPinsForWrite();
  byte lowReadAddress = address & 0xFF;
  byte highReadAddress = (address >> 8) & 0xFF;
  core_setDataPinsValue(lowReadAddress);
  core_latchLowAddress();
  core_setDataPinsValue(highReadAddress);
  core_latchHighAddress();
  assertRead();
  selectRAM();
  setupDataPinsForRead();
  return readData();
}

void setChipSelect(uint8_t cs){
  if(cs == 0 ){
    controlStatus = core_clearControlBit(CONTROL_SEL_A, controlStatus);
    controlStatus = core_clearControlBit(CONTROL_SEL_B, controlStatus);
  }
  if(cs & 1 == 1){
    controlStatus = core_setControlBit(CONTROL_SEL_A, controlStatus);
  }
  if(cs & 2 == 2 ){
    controlStatus = core_setControlBit(CONTROL_SEL_B, controlStatus);
  }
}

int writeFileToSRAM(const file_t &romFile){
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
    chipSelect = 2;
  }

  if(romSize >=64000){
    offset = 0;
    chipSelect = 3;
  }
  setChipSelect(chipSelect);
  setupDataPinsForWrite();
  Serial.print("\nStarting ROM Read with Offset ");
  Serial.print(offset, HEX);
  Serial.print(" and size ");
  Serial.print(romSize);
  Serial.print("kb");
  long int startTime = millis();
  unsigned int address = 0;
  
  assertWrite();
  romFile.seek(0);
  while (romFile.available()) {
    unsigned int offsetAddress = address + offset;
    byte lowAddress = offsetAddress & 0xFF;
    byte highAddress = (offsetAddress >> 8) & 0xFF;
    byte data = romFile.read();
    core_setDataPinsValue(lowAddress);
    core_latchLowAddress();
    core_setDataPinsValue(highAddress);
    core_latchHighAddress();
    core_setDataPinsValue(data);
    selectRAM();
    deselectRAM();
    address++;
  }
  long int completeTime = millis();
  Serial.print("\nCompleted ROM Read in ");
  Serial.print(completeTime - startTime);
  Serial.print(" ms");
  Serial.print("\nHanding over to MSX, using signal ");Serial.print(chipSelect);Serial.print("\n");  
}


void createUnsortedFileListFile(sd_t sd, file_t& directory, char* filename){
  // Started: 2023/04/29
  // Will finish this later
  // I want to have the file list stored in this file, so that we don't need to constantly
  // iterate over the SD Card Hierarchy to get the file list.
  // This will be the unsorted version, and then we will have a sorted file list, so that we can display
  // the files sorted, which we wouldnt be able to do otherwise, since we do not have the RAM to store
  // all the file names, especially assuming people have 100's of files.
  // The sorting should only happen if there is a difference, but if this is impossible to do, we would just
  // do it everytime the system is booted.
  file_t indexFile;
  indexFile.open(filename, O_WRITE);
  indexFile.close();
}


