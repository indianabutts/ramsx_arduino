#include "sd_utilities.h"

sd_t sd_initializeSDCard(int pin) {
  Serial.print("Initializing SD card...\r\n");
  sd_t sd;

  if (!sd.begin(pin, SPI_FULL_SPEED)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    sd.initErrorHalt();
  }
  Serial.print("\nSD Card Initialized");
  return sd;
}

void sd_getNFilesFromOffset(sd_t sd, file_t& directory, uint8_t fileCount, uint16_t pageNumber){
  // A Page is fileCount in size, so assuming fileCount of 21, and page number 1, this would offset by 21
  file_t file;
  char fileName[20];

  if(!directory.isDir()) {
    Serial.println("Not a valid Directory, exiting from function");
    return;
  }
  directory.rewindDirectory();
  for(unsigned int i=0; i< fileCount*pageNumber; i++){
    file.openNext(&directory, O_READ);
    file.close();
  }
}

void sd_displayDirectoryContent(sd_t sd, file_t& aDirectory, byte tabulation) {
  file_t file;
  char fileName[255];

  if (!aDirectory.isDir()) return;
  aDirectory.rewindDirectory();

  unsigned int counter = 0;
  file.getName(fileName, sizeof(fileName));
  Serial.print("\n\n Listing for DIR: "); Serial.print(fileName);Serial.print("\n=================================================\n");
  Serial.print("File Name                    \t Size \t Start Address\n");
  Serial.print("=================================================\n");
  while (file.openNext(&aDirectory, O_READ)) {
    if (!file.isHidden()) {
      file.getName(fileName, sizeof(fileName));
      fileName[20] = '\0';
      for (uint8_t i = 0; i < tabulation; i++) Serial.write('\t');
      if (file.isDir()) {
        Serial.println(F("/"));Serial.print(fileName);
      } else {
        file.seek(3);
        byte msb = file.peek();
        file.seek(2);
        byte lsb = file.peek();
        Serial.print(fileName);Serial.write('\t'); Serial.print((uint32_t) file.fileSize()/1000); Serial.print(" kb \t"); Serial.print("0x");Serial.print(msb,HEX),Serial.print(lsb,HEX);Serial.print("\n");
      }
    }
    counter ++;
    file.close();
  }
  Serial.print("\n ");Serial.print(counter);Serial.print(" total files in directory");
}