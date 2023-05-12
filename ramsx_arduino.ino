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

int main() {

  
  core_initializeControlPins();
  control_haltMSX();
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  
  // core_initializeDataPinsForWrite();
  
  // controlStatus = control_setControlForBootloaderWrite(controlStatus);

  
  sd_t sd = sd_initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  // file_t romFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }

  char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // programROM(&sd, &root, filename);
  programBootloader(sd, root);
}


void programROM(sd_t& sd, file_t& root, char* filename){
  core_initializeDataPinsForWrite();
  controlStatus = control_setControlForROMWrite(controlStatus);

  file_t romFile;
  
  if(!romFile.open(&root, filename, O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }

  controlStatus = core_writeFileToSRAM(romFile, controlStatus); 




  controlStatus = control_handover(controlStatus);
  controlStatus=control_clearReadAndWrite(controlStatus);
  control_releaseMSX();
}


void programBootloader(sd_t& sd, file_t& root){
  
  core_initializeDataPinsForWrite();
  
  controlStatus = control_setControlForBootloaderWrite(controlStatus);

  
  file_t romFile;
  if(!romFile.open(&root, "out.rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  controlStatus = core_writeFileToSRAM(romFile, controlStatus); 

  sd_seekToFileOffset(sd, root, SD_DEFAULT_FILE_COUNT, 0);
  for(int i=0; i<SD_DEFAULT_FILE_COUNT; i++){
    SD_RomFile testFile = sd_getFileFromOffset(sd, root);
    // Serial.println(testFile.fileName);
    for (int j=0; j<SD_DEFAULT_FILE_NAME_SIZE+1; j++){
      uint16_t address = 0x4010+i*(SD_DEFAULT_FILE_NAME_SIZE+1)+j;
      uint8_t data;
      if(j==SD_DEFAULT_FILE_NAME_SIZE){
        data = 0;
      } else if (j>=0 && j<21) {
        data = testFile.fileName[j];
      } else if (j>=21 && j<22){
        data = ' ';
      } else if (j>=23){
        char fileSizeBuffer[6];
        char fileSizeString[4];
        sprintf(fileSizeString, "%3d", testFile.fileSize/1000);
        sprintf(fileSizeBuffer, "%3skb",fileSizeString);
        data =fileSizeBuffer[j-23]; 
      }
      
      core_writeDataToAddress(address, data);
    }
  }

  
  controlStatus = control_handover(controlStatus);
  controlStatus=control_clearReadAndWrite(controlStatus);
  control_releaseMSX();
  
  bool commandHandled = false;
  // while (true){
  //   if(core_checkForCommandSignal()){
  //     Serial.println("Command Triggered");
  //     control_haltMSX();
  //     controlStatus = control_takeover(controlStatus);
  //     controlStatus = core_readDataFromAddress(&data, controlStatus, 0x6FFF);
  //     Serial.println(data);
  //     controlStatus = control_assertWrite(controlStatus);
  //     core_writeDataToAddress(0x6FFE, 0x6F);
  //     controlStatus = control_handover(controlStatus);
  //     controlStatus=control_clearReadAndWrite(controlStatus);

  //     control_releaseMSX();
  //   }
    
  
  // }
}

