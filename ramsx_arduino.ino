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
uint8_t currentPage = 0;

void setup() {
  core_initializeControlPins();
  control_haltMSX();

  Serial.begin(115200);
  Serial.print(F("\n******* Starting New Run *******\n"));
  
  
  
  sd_t sd = sd_initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  file_t indexFile;
  if(!root.open("/")){
    Serial.print(F("\nERROR: Error Opening Dir"));
    sd.errorHalt(&Serial);
  }
  if(!root.exists("testIndex.txt")){
    Serial.println(F("Index File does not exist - Building now"));
    indexFile.open(&root,"testIndex.txt", FILE_WRITE);
    sd_buildIndexFile(sd, root, indexFile);
    indexFile.close();
  } else {
    Serial.println(F("Index File Exists, Accessing File"));
    indexFile.open(&root,"testIndex.txt", O_RDONLY);    
  }
  
  char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  
  programBootloader(sd, root, indexFile);
  Serial.print(F("\n******* Completed Run *******\n"));
  
}


void programROM(sd_t& sd, file_t& root, char* filename){
  core_initializeDataPinsForWrite();
  controlStatus = control_setControlForROMWrite(controlStatus);

  file_t romFile;
  
  if(!romFile.open(&root, filename, O_READ)){
    Serial.print(F("\nERROR: Error Opening File"));
    sd.errorHalt(&Serial);
  }

  controlStatus = core_writeFileToSRAM(romFile, controlStatus); 




  // controlStatus = control_handover(controlStatus);
  // controlStatus=control_clearReadAndWrite(controlStatus);
  // control_releaseMSX();
}


void programBootloader(sd_t& sd, file_t& root, file_t& indexFile){
  Serial.println(F("Programming Bootloader"));
  core_initializeDataPinsForWrite();
  file_t rom;
  if(!rom.open(&root, "out.rom", O_RDONLY)){
     Serial.print(F("\nERROR: Error Opening File"));
    sd.errorHalt(&Serial);
  };
  controlStatus = control_setControlForBootloaderWrite(controlStatus);

  
 
  controlStatus = core_writeFileToSRAM(rom, controlStatus); 

  uint8_t totalPages = (sd_totalFilesInDirectory(indexFile)+20)/21;
    
  char totalPagesString[4];
  sprintf(totalPagesString, "%-3d", totalPages);
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

  core_writeDataToAddress(0x43A8+32, totalPagesString[0]);
  core_writeDataToAddress(0x43A8+33, totalPagesString[1]);
  core_writeDataToAddress(0x43A8+34, totalPagesString[2]);
  // core_writeDataToAddress(0x6FFE, 0xA2);
  
  controlStatus = control_handover(controlStatus);
  // controlStatus=control_clearReadAndWrite(controlStatus);
  control_releaseMSX();
  uint8_t data;
  bool programCommand = false;
  delay(1000);
  bool commandState = false;
  bool lastCommandState = false;
  while (!programCommand){
    commandState = core_checkForCommandSignal();
    // Serial.print(commandState); Serial.print(F(", L: ")); Serial.print(lastCommandState);Serial.print(F("\n"));
    if((commandState!=lastCommandState) && commandState){
      controlStatus = control_takeover(controlStatus);
      Serial.println(F("Command Triggered"));
      control_haltMSX();
      controlStatus = core_readDataFromAddress(&data, controlStatus, 0x6FF0);
      if(data == 0x60){
        programCommand=true;
        uint8_t romIndex;
        controlStatus = core_readDataFromAddress(&romIndex, controlStatus, 0x6FF1);
        

        SD_RomFile selectedFile = sd_getFilenameFromOffset(indexFile, currentPage, 21, romIndex);
        Serial.print(F("Program ROM Command Issued for ")); Serial.print(selectedFile.fileName);
        programROM(sd, root, selectedFile.fileName);
        // programROM(sd, root, filename);

      }
      core_initializeDataPinsForWrite();
      Serial.println(data, HEX);
      controlStatus = control_assertWrite(controlStatus);
      core_writeDataToAddress(0x6FFE, 0xA2);    
      controlStatus = control_handover(controlStatus);
      control_releaseMSX();
      // delay(1000);
      // controlStatus = control_clearCommandFlag(controlStatus);
    }
    lastCommandState = commandState;
  
  }
}

void loop(){}

