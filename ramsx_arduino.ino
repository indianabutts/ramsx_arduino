#include <SD.h>

#define DATA_START  2
#define DATA_END  9

#define nWRITE  A0
#define nREAD A1
#define CON_A A2
#define CON_B  A3
#define CON_C  A4
#define CON_DATA A5

void setup() {

  
  Serial.begin(9600);
  Serial.print("\n******* Starting New Run *******\n");
  while(!Serial)
    ;
  
  initializeSDCard(10);
  setupPinsForWrite();

  File romFile = readFileFromSDCard("/KNIGHT.ROM");

  romFile.seek(3);

  unsigned int offset = 0;
  char* chipSelect = "CS1";
  if(romFile.peek() >= 0x80){
    offset = 0x8000;
    chipSelect = "CS2";
  } else if (romFile.peek() >= 0x40 && romFile.peek()<0x80){
    offset = 0x4000;
  }

  if(romFile.size()>=32000){
    chipSelect = "CS12";
  }

  Serial.print("\nStarting ROM Read with Offset ");Serial.print(offset, HEX);
  long int startTime = millis(); 
  unsigned int address = 0;
  assertReset();
  assertWrite();
  romFile.seek(0);
  while(romFile.available()){
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
    address ++;
  }
  long int completeTime = millis();

  Serial.print("\nCompleted ROM Read in "); Serial.print(completeTime-startTime); Serial.print(" ms");
  Serial.print("\nHanding over to MSX, use signal "); Serial.print(chipSelect);
  handover();
  while(true)
    ;
}

void loop() {
  // put your main code here, to run repeatedly:

}

void setupPinsForWrite(){
  
  for(int i = DATA_START; i<=DATA_END; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(nREAD, OUTPUT);
  pinMode(nWRITE, OUTPUT);
  pinMode(CON_A, OUTPUT);
  pinMode(CON_B, OUTPUT);
  pinMode(CON_C, OUTPUT);
  pinMode(CON_DATA, OUTPUT);
}
void setDataPinValue(byte data){
  // Serial.println(data);
  // Put lower 6 bits onto port D2-7
  // Put upper 8 bits onto port B0-1
  for(unsigned int i = DATA_START; i<=DATA_END; i++){
    unsigned int currentBit = i-2;
    digitalWrite(i, bitRead(data, currentBit));
  }
}

void initializeSDCard(int pin) {
  Serial.print("Initializing SD card...");

  if (!SD.begin(pin)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true)
      ;
  }
}
void latchData(){
  digitalWrite(CON_DATA, 1);  
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 1);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void latchLowAddress(){
  digitalWrite(CON_DATA, 1);
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 1);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void latchHighAddress(){
  digitalWrite(CON_DATA, 1);  
  digitalWrite(CON_A, 1);
  digitalWrite(CON_B, 1);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void handover(){
  digitalWrite(nREAD, 1);
  digitalWrite(nWRITE, 1);
  digitalWrite(CON_DATA, 1);
  digitalWrite(CON_A, 1);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 1);
  digitalWrite(CON_DATA, 0);
}

void selectRAM(){
  digitalWrite(CON_DATA, 1);
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 0);
}

void deselectRAM(){
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 1);
}

void assertWrite(){
  digitalWrite(nWRITE, 0);
  digitalWrite(nREAD, 1);
}
void assertReset(){
  digitalWrite(CON_A, 1); 
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 1);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}
File readFileFromSDCard(char* fileName) {
  return SD.open(fileName);
}