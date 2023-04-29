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

#define CONTROL_A16 0
#define CONTROL_nREAD 1
#define CONTROL_nWRITE 2
#define CONTROL_SEL_A 3
#define CONTROL_SEL_B 4
#define CONTROL_HANDOVER 5
#define CONTROL_COMM_RESET 6


uint8_t controlStatus = 0;

void setup() {

  setupControlPins();
  haltMSX();
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  
  setupDataPinsForWrite();
  
  setControlForBootloaderWrite();

  
  sd_t sd = initializeSDCard(10);
  sd.setDedicatedSpi(true);
  
  
  file_t root;
  file_t romFile;
  if(!root.open("/")){
    Serial.print("\nERROR: Error Opening Dir");
    sd.errorHalt(&Serial);
  }
  // char* filename = "Tank Battalion (1984)(Namcot)(JP).rom";
  // char* filename = "testrom_write.rom";
  if(!romFile.open(&root, "out.rom", O_READ)){
    Serial.print("\nERROR: Error Opening File");
    sd.errorHalt(&Serial);
  }
  writeFileToSRAM(romFile); 
  handover();
  releaseMSX();
  clearReadOrWrite();
  while (true);

}

void loop() {
}


void latchControl(){
  PORTC |= B001000;
  PORTC &= B110111;
}

void latchLowAddress() {
  PORTC |= B000001 ;
  PORTC &= B111110 ;
}

void latchHighAddress() {
  PORTC |= B000010;
  PORTC &= B111101;
}

void setControlForBootloaderWrite(){
  Serial.println("Setting up for Bootloader Write");
  clearControlBit(CONTROL_A16);
  clearControlBit(CONTROL_nWRITE);
  clearControlBit(CONTROL_COMM_RESET);
  setControlBit(CONTROL_nREAD);
  setControlBit(CONTROL_HANDOVER);
}

void haltMSX(){
  PORTC &= B101111;
}

void releaseMSX(){
  PORTC |= B010000;
}

void deassertAddress16(){
  Serial.println("\n Deasserting A16");
  clearControlBit(CONTROL_A16);
}

void assertAddress16(){
  Serial.println("\n Asserting A16");
  setControlBit(CONTROL_A16);
}


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
void setDataPinsValue(byte data) {
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
}


sd_t initializeSDCard(int pin) {
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



void setControlBit(uint8_t controlPin){
  // Serial.print("Setting Control Bit "); Serial.print(controlPin);Serial.print("\r\n");
  // Serial.print("Previous Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
  controlStatus |= (1<<controlPin);
  setDataPinsValue(controlStatus);
  latchControl();
  // Serial.print("New Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
}
void clearControlBit(uint8_t controlPin){
  // Serial.print("Clearing Control Bit "); Serial.print(controlPin);Serial.print("\r\n");
  // Serial.print("Previous Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
  controlStatus &= ~(1UL<<controlPin);
  setDataPinsValue(controlStatus);
  latchControl();
  // Serial.print("New Control: B"); Serial.print(controlStatus, BIN);Serial.print("\r\n");
}

void handover() {
  Serial.println("\nHanding Over");
  clearControlBit(CONTROL_HANDOVER);
}

void selectRAM() {
  PORTC &= B111011;
}

void deselectRAM() {
  PORTC |= B000100;
}

void assertWrite() {
  Serial.println("\nAsserting Write");
  setControlBit(CONTROL_nREAD);
  clearControlBit(CONTROL_nWRITE);
}

void assertRead() {
  Serial.println("\nAsserting Read");
  clearControlBit(CONTROL_nREAD);
  setControlBit(CONTROL_nWRITE);
}

uint8_t readData(){
  byte data = (PIND & B11111100)>>2;
  return data | (((PINB & B00000011))<<6);
}

void clearReadOrWrite(){
  Serial.println("\nClearing Read and Write");
  setControlBit(CONTROL_nREAD);
  setControlBit(CONTROL_nWRITE);
}


uint8_t readDataFromAddress(uint16_t address){
  deselectRAM();
  //deassertReadandWrite();
  setupDataPinsForWrite();
  byte lowReadAddress = address & 0xFF;
  byte highReadAddress = (address >> 8) & 0xFF;
  setDataPinsValue(lowReadAddress);
  latchLowAddress();
  setDataPinsValue(highReadAddress);
  latchHighAddress();
  assertRead();
  selectRAM();
  setupDataPinsForRead();
  return readData();
}

void setChipSelect(uint8_t cs){
  if(cs == 0 ){
    clearControlBit(CONTROL_SEL_A);
    clearControlBit(CONTROL_SEL_B);
  }
  if(cs & 1 == 1){
    setControlBit(CONTROL_SEL_A);
  }
  if(cs & 2 == 2 ){
    setControlBit(CONTROL_SEL_B);
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
    setDataPinsValue(lowAddress);
    latchLowAddress();
    setDataPinsValue(highAddress);
    latchHighAddress();
    setDataPinsValue(data);
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
