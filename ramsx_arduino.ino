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
#include "control.h"

uint8_t controlStatus = 0;

void setup() {

  
  core_initializeControlPins();
  control_haltMSX();
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  
  core_initializeDataPinsForWrite();
  
  controlStatus = control_setControlForBootloaderWrite(controlStatus);

  
  sd_t sd = sd_initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  file_t romFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }
  // char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // char* filename = "testrom_write.rom";
  if(!romFile.open(&root, "write.rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  controlStatus = control_assertAddress16(controlStatus);
  controlStatus = core_writeFileToSRAM(romFile, controlStatus); 

  // seekToFileOffset(sd, root, SD_DEFAULT_FILE_COUNT, 0);
  // for(int i=0; i<SD_DEFAULT_FILE_COUNT; i++){
  //   SD_RomFile testFile = sd_getFileFromOffset(sd, root);
  //   // Serial.println(testFile.fileName);
  //   for (int j=0; j<SD_DEFAULT_FILE_NAME_SIZE+1; j++){
  //     uint16_t address = 0x4010+i*(SD_DEFAULT_FILE_NAME_SIZE+1)+j;
  //     uint8_t data;
  //     if(j==SD_DEFAULT_FILE_NAME_SIZE){
  //       data = 0;
  //     } else {
  //       data = testFile.fileName[j];
  //     }
      
  //     core_writeDataToAddress(address, data);
  //   }
  // }

  // TODO [FIX]: Read needs both the Data of the Arduino Pins, and 
  // Serial.println("CONTROL BEFORE READ");
  // Serial.println(controlStatus, BIN);
  // uint8_t data = 0;
  // for(int i =0; i< 25; i++){
  //   uint16_t currentAddress = 0x4000 + i;
  //   controlStatus = core_readDataFromAddress(&data, controlStatus, 0x4000+i);
  //   char buffer[17];
  //   sprintf(buffer, "A:0x%04x D: 0x%02x", currentAddress, data);
  //   Serial.println(buffer);
  // }

  core_initializeDataPinsForWrite();

  controlStatus = control_handover(controlStatus);
  controlStatus=control_clearReadAndWrite(controlStatus);
  control_releaseMSX();
  
  while (true){
      if(core_checkForCommandSignal()){
        controlStatus = control_assertWrite(controlStatus);
        core_WriteDataToAddress(0x6FFE, 0x6F);
        controlStatus = control_clearReadAndWrite(controlStatus);
      } 
  }
}

void loop() {
}







void createUnsortedFileListFile(sd_t sd, file_t& directory, char* filename){

  file_t indexFile;
  indexFile.open(filename, O_WRITE);
  indexFile.close();
}


