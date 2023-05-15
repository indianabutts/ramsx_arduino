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
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  
  
  
  sd_t sd = sd_initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  // file_t indexFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }
  if(!root.exists("testIndex.txt")){
    Serial.println("Index File does not exist - Building now");
    // indexFile.open(&root,"testIndex.txt", FILE_WRITE);
    // sd_buildIndexFile(sd, root, indexFile);
    // indexFile.close();
  } else {
    Serial.println("Index File Exists, Accessing File");
    // indexFile.open(&root,"testIndex.txt", O_RDONLY);    
  }
  
  // bootloader.open(&root, "out.rom", O_READ);
  // SD_RomFile currentFile = sd_getFilenameFromOffset(indexFile, 0, 21,0);
  // Serial.println(currentFile.fileName);
  // indexFile.close();
  
  // if(!indexFile.open(&root, "out.rom", O_READ)){
  //   Serial.print("\nERROR: Error Opening File");
  //   sd.errorHalt(&Serial);
  // }


  char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // char* filename = "vram.rom";
  programROM(sd, root, filename);
  // programBootloader(sd, indexFile);
  while(true);
  Serial.print("\n******* Completed Run *******\n");
  
}

void loop(){}
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


void programBootloader(sd_t& sd, file_t& indexFile){
  Serial.println("Programming Bootloader");
  core_initializeDataPinsForWrite();
  indexFile.close();
  if(!indexFile.open("out.rom", O_RDONLY)){
     Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  };
  controlStatus = control_setControlForBootloaderWrite(controlStatus);

  
 
  controlStatus = core_writeFileToSRAM(indexFile, controlStatus); 
  indexFile.close();
  indexFile.open("testIndex.txt");

  uint8_t totalPages = (sd_totalFilesInDirectory(indexFile)+20)/21;
  // sd_getFilenameFromOffset(indexFile, 1, 21,3);
    

  for(int i=0; i<21; i++){
    SD_RomFile currentFile = sd_getFilenameFromOffset(indexFile, 0, 21,i);
    char fileSizeBuffer[6];
    char fileSizeString[4];
    sprintf(fileSizeString, "%3d", currentFile.fileSize);
    sprintf(fileSizeBuffer, "%3skb",fileSizeString);
    for (int j=0; j<31; j++){
      uint16_t address = 0x4060+i*40+j;
      uint8_t data;
      if((j>20 && j<24) | j==25){
        data = ' ';
      } else if (j>0 && j<21){
        data = currentFile.fileName[j-1];
      } else if (j>=26){
        data =fileSizeBuffer[j-26]; 
      }
      if(j!=24 && j!=0){
        core_writeDataToAddress(address, data);
      }
    }
  }

  
  controlStatus = control_handover(controlStatus);
  controlStatus=control_clearReadAndWrite(controlStatus);
  control_releaseMSX();
  
  // bool commandHandled = false;
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

