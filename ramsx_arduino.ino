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

  File romFile = readFileFromSDCard("/TANKBA~1.ROM");

  Serial.println("Starting ROM Read");
  long int startTime = millis(); 
  unsigned int address = 0;
  assertReset();
  assertWrite();
  while(romFile.available()){
    unsigned int offsetAddress = address + 0x4000;
    byte lowAddress = offsetAddress & 0xFF;
    byte highAddress = (offsetAddress >> 8) & 0xFF;
    byte data = romFile.read();
    setDataPinValue(lowAddress);
    latchLowAddress();
    setDataPinValue(highAddress);
    latchHighAddress();
    setDataPinValue(data);
    latchData();
    enableRAM();
    address ++;
  }
  long int completeTime = millis();

  Serial.print("\nCompleted ROM Read in "); Serial.print(completeTime-startTime); Serial.print(" ms");
  Serial.println("\nHanding over to MSX");
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
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 1);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void latchLowAddress(){
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 1);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void latchHighAddress(){
  digitalWrite(CON_A, 1);
  digitalWrite(CON_B, 1);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void handover(){
  digitalWrite(CON_A, 1);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 1);
  digitalWrite(CON_DATA, 0);
}

void enableRAM(){
  digitalWrite(CON_A, 0);
  digitalWrite(CON_B, 0);
  digitalWrite(CON_C, 0);
  digitalWrite(CON_DATA, 1);
  digitalWrite(CON_DATA, 0);
  digitalWrite(CON_DATA, 1);
}

void assertWrite(){
  digitalWrite(nWRITE, 0);
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
  return SD.open("/BINARY.ROM");
}