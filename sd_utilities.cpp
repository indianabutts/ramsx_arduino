#include "sd_utilities.h"
#include <string.h>

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
  // Serial.print("\nSD Card Initialized");
  return sd;
}
void sd_seekToFileOffset(sd_t& sd, file_t& directory, uint8_t fileCount, uint16_t pageNumber){
  file_t file;
  if(!directory.isDir()) {
    Serial.println("Not a valid Directory, exiting from function");
    return;
  }
  
  directory.rewindDirectory();

  //TODO: Need to find a better way to achieve this, for now, I only know this way of seeking to the correct offset....
  for(unsigned int i=0; i< fileCount*pageNumber; i++){
    file.openNext(&directory, O_READ);
    file.close();
  }
}

SD_RomFile sd_getFileFromOffset(sd_t& sd, file_t& directory){
  // A Page is fileCount in size, so assuming fileCount of 21, and page number 1, this would offset by 21
  file_t file;
  char fileName[255];
  SD_RomFile currentFile;
  if(!directory.isDir()) {
    Serial.println("Not a valid Directory, exiting from function");
    return;
  }  
  
  file.openNext(&directory, O_RDONLY);
  file.getName(fileName, sizeof(fileName));
  fileName[SD_DEFAULT_FILE_NAME_SIZE]='\0';
  char truncatedString[SD_DEFAULT_FILE_NAME_SIZE];
  strncpy(currentFile.fileName, fileName, SD_DEFAULT_FILE_NAME_SIZE);
  
  file.seek(3);
  uint16_t msb = file.peek();
  file.seek(2);
  uint16_t lsb = file.peek();
  currentFile.offset = (msb<<8) | lsb;
  currentFile.fileSize = file.fileSize();
  file.close();
  return currentFile;
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
  return;
}

uint16_t sd_totalFilesInDirectory(file_t& indexFile){
  indexFile.seekSet(indexFile.fileSize()-sizeof(char)*117);
  char countBuffer[10];
  indexFile.read(countBuffer,4);
  return atoi(countBuffer);
}

void sd_buildIndexFile(sd_t sd, file_t& directory, file_t& indexFile){
  // Started: 2023/04/29
  // Will finish this later
  // I want to have the file list stored in this file, so that we don't need to constantly
  // iterate over the SD Card Hierarchy to get the file list.
  // This will be the unsorted version, and then we will have a sorted file list, so that we can display
  // the files sorted, which we wouldnt be able to do otherwise, since we do not have the RAM to store
  // all the file names, especially assuming people have 100's of files.
  // The sorting should only happen if there is a difference, but if this is impossible to do, we would just
  // do it everytime the system is booted.
  file_t file;
  
  char fileName[100];
  char offsetValue[7];
  char fileSize[6];
  char lineBuffer[120];
  if (!directory.isDir()) return;
  directory.rewindDirectory();

  unsigned int counter = 0;
  while (file.openNext(&directory, O_READ)) {
    if (!file.isHidden()) {
      
      file.getName(fileName, sizeof(fileName));
      
      // fileName[strlen(fileName)]='\0';
      if (file.isDir()) {
        
      } else {
        file.seek(3);
        byte msb = file.peek();
        file.seek(2);
        byte lsb = file.peek();
        
        uint16_t offset = (msb<<8) | lsb;
        sprintf(offsetValue, "0x%04x", offset);
        // sprintf(lineBuffer, "%d", counter);
        // Serial.println(lineBuffer);
        sprintf(fileSize, "%3d", (uint32_t)file.fileSize()/1000);
        // sprintf(lineBuffer, "%d", (uint32_t)file.fileSize()/1000);
        sprintf(lineBuffer, "%4d:%-100s:%6s:%3s\n", counter, fileName, offsetValue, fileSize);
        indexFile.write(lineBuffer);
        indexFile.sync();
        // Serial.println(lineBuffer);
        // sprintf(lineBuffer, "%d,%s,%d, %d", counter, "Test", fileSize, offset);
        // Serial.println(lineBuffer);
        
      }
      counter ++;
    }
    file.close();
  }
  return;
}
SD_RomFile sd_getFilenameFromOffset(file_t& indexFile, uint8_t pageNumber, uint8_t pageEntries, uint8_t offset){
  uint32_t seekPosition = (uint32_t)((uint32_t)pageNumber*(uint32_t)pageEntries+(uint32_t)offset)*(uint32_t)sizeof(char)*(uint32_t)117;
  indexFile.seek(seekPosition);
  char line[120];
  char *token;
    // 21,Sparkie (1983)(Sony)(JP).rom, 0x00cf,  8
    
  SD_RomFile currentFile;
  // indexFile.read(line, 118*sizeof(char));
  indexFile.fgets(line, 117*sizeof(char));
  Serial.println(line);
  token = strtok(line,":");
  token = strtok(NULL,":");
  sd_remove_space(token);
  strcpy(currentFile.fileName,token);
  token = strtok(NULL,":");
  currentFile.offset = (uint16_t)strtol(token,NULL,0);
  token = strtok(NULL,":");
  currentFile.fileSize = atoi(token);
  return currentFile;
}  

void sd_remove_space(char* s){
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*s)) s++;

  if(*s == 0)  // All spaces?
    return s;

  // Trim trailing space
  end = s + strlen(s) - 1;
  while(end > s && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return s;
}