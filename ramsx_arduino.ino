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

char* testName = "YASSIR ALREFAIE.     ";
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

  // sd_displayDirectoryContent(sd, root, 0);
  // while(true);
  // char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // char* filename = "testrom_write.rom";
  if(!romFile.open(&root, "out.rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  controlStatus = core_writeFileToSRAM(romFile, controlStatus); 

  seekToFileOffset(sd, root, 21, 0);
   SD_RomFile testFile = 
  controlStatus = control_handover(controlStatus);
  control_releaseMSX();
  controlStatus=control_clearReadAndWrite(controlStatus);
  while (true);
}

void loop() {
}







void createUnsortedFileListFile(sd_t sd, file_t& directory, char* filename){

  file_t indexFile;
  indexFile.open(filename, O_WRITE);
  indexFile.close();
}


