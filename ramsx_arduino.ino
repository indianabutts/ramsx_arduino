#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <RingBuf.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2

#if SD_FAT_TYPE == 0
typedef SdFat sd_t;
typedef File file_t;
#elif SD_FAT_TYPE == 1
typedef SdFat32 sd_t;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
typedef SdExFat sd_t;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
typedef SdFs sd_t;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE



void setup() {
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  while (!Serial)
    ;

  sd_t sd = initializeSDCard(10);
  sd.setDedicatedSpi(true);
  Serial.print("\nSD Card Initialized");
  
  file_t root;
  file_t romFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }
  if(!romFile.open(&root, "Tank Battalion (1984)(Namcot)(JP).rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  writeFileToSRAM(romFile);
  // displayDirectoryContent(sd, root, 0);
  handover();
  while (true);


}

void loop() {
  // file_t dir;

  // if(!dir.open("/")){
  //   Serial.print("\n[ERROR] Error opening Root Dir");
  //   sd.errorHalt(&Serial);
  // }
  // setupPinsForWrite();
  // // put your main code here, to run repeatedly:
}

void setupPinsForWrite() {
  DDRD = DDRD | B11111100;
  DDRB = DDRB | B00000011;
  DDRC = DDRC | B111111;
  // Serial.println("\nSetting Port D2-7, B0-1, C0-5 as Outputs");
}
void setupPinsForRead() {
  DDRD = DDRD & B00000011;
  DDRB = DDRB & B11111100;
  DDRC = DDRC | B111111;
  // Serial.println("\nSetting Port D2-7, B0-1 as Input, C0-5 as Outputs");
}
void setDataPinValue(byte data) {
  // Serial.println(data);
  // Put lower 6 bits onto port D2-7
  // Put upper 8 bits onto port B0-1
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
}

int writeFileToSRAM(const file_t &romFile){
  uint32_t romSize = (uint32_t) romFile.fileSize();

  romFile.seek(3);

  unsigned int offset = 0;
  char* chipSelect = "CS1";
  bool needToSwap = false;
  byte peekValue = romFile.peek();
  if (peekValue >= 0x80) {
    offset = 0x8000;
    chipSelect = "CS2";
  } else if (peekValue >= 0x40 && peekValue < 0x80) {
    offset = 0x4000;
  }

  if (romSize >= 32000) {
    chipSelect = "CS12";
  }

  if(romSize >=64000){
    offset = 0;
  }
  setupPinsForWrite();
  Serial.print("\nStarting ROM Read with Offset ");
  Serial.print(offset, HEX);
  Serial.print(" and size ");
  Serial.print(romSize);
  Serial.print("kb");
  long int startTime = millis();
  unsigned int address = 0;
  assertReset();
  assertWrite();
  romFile.seek(0);
  while (address < romSize) {
    unsigned int offsetAddress = address + offset;
    byte lowAddress = offsetAddress & 0xFF;
    byte highAddress = (offsetAddress >> 8) & 0xFF;
    byte data = romFile.read();
    setDataPinValue(lowAddress);
    latchLowAddress();
    setDataPinValue(highAddress);
    latchHighAddress();
    setDataPinValue(data);
    latchData();
    selectRAM();
    deselectRAM();
    address++;
  }
  long int completeTime = millis();
  Serial.print("\nCompleted ROM Read in ");
  Serial.print(completeTime - startTime);
  Serial.print(" ms");
  Serial.print("\nHanding over to MSX, use signal ");Serial.print(chipSelect);Serial.print("\n");


}

void displayDirectoryContent(sd_t sd, file_t& aDirectory, byte tabulation) {
  file_t file;
  char fileName[255];

  if (!aDirectory.isDir()) return;
  aDirectory.rewind();
  unsigned int counter = 0;
  file.getName(fileName, sizeof(fileName));
  Serial.print("\n\n Listing for DIR: "); Serial.print(fileName);Serial.print("\n=================================================\n");
  Serial.print("File Name                    \t Size \t Start Address\n");
  Serial.print("=================================================\n");
  while (file.openNext(&aDirectory, O_READ)) {
    if (!file.isHidden()) {
      file.getName(fileName, sizeof(fileName));
      fileName[30] = '\0';
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

sd_t initializeSDCard(int pin) {
  Serial.print("Initializing SD card...");
  sd_t sd;

  if (!sd.begin(pin, SPI_FULL_SPEED)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    sd.initErrorHalt();
  }

  return sd;
}
void latchData() {
  PORTC = (PORTC & 0x03) | B010000;
  PORTC = (PORTC & 0x03) | B110000;
}

void latchLowAddress() {
  PORTC = (PORTC & 0x03) | B001000;
  PORTC = (PORTC & 0x03) | B101000;
}

void latchHighAddress() {
  PORTC = (PORTC & 0x03) | B001100;
  PORTC = (PORTC & 0x03) | B101100;
}

void handover() {
  PORTC = (PORTC & 0x00) | B010111;
}

void selectRAM() {
  PORTC = (PORTC & 0x03) | B000000;
}

void deselectRAM() {
  PORTC = (PORTC & 0x03) | B100000;
}

void assertWrite() {
  PORTC = (PORTC & B111100) | B000010;
}

void assertRead() {
  PORTC = (PORTC & B111100) | B000001;
}

uint8_t readData(){
  byte data = (PIND & B11111100)>>2;
  return data | (((PINB & B00000011))<<6);
}


void assertReset() {
  PORTC = (PORTC & 0x03) | B000100;
  PORTC = (PORTC & 0x03) | B100100;
}

uint8_t readDataFromAddress(uint16_t address){
  deselectRAM();
  deassertReadandWrite();
  setupPinsForWrite();
  byte lowReadAddress = address & 0xFF;
  byte highReadAddress = (address >> 8) & 0xFF;
  setDataPinValue(lowReadAddress);
  latchLowAddress();
  setDataPinValue(highReadAddress);
  latchHighAddress();
  assertRead();
  selectRAM();
  setupPinsForRead();
  return readData();
}

void deassertReadandWrite(){
  PORTC = (PORTC & B111100) | B000011;
}


// File readFileFromSDCard(char* fileName) {
//   return SD.open(fileName);
// }