#include <SD.h>

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
  DDRD = B11111100;
  DDRB = DDRB | B00000011;
  DDRC = DDRC | B111111;
  Serial.println("\nSetting Port D2-7, B0-1, C0-5 as Outputs");
}
void setDataPinValue(byte data){
  // Serial.println(data);
  // Put lower 6 bits onto port D2-7
  // Put upper 8 bits onto port B0-1
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3 ;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
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
  PORTC = (PORTC & 0x03) | B010000;
  PORTC = (PORTC & 0x03) | B110000;
  PORTC = (PORTC & 0x03) | B010000;
}

void latchLowAddress(){
  PORTC = (PORTC & 0x03) | B001000;
  PORTC = (PORTC & 0x03) | B101000;
  PORTC = (PORTC & 0x03) | B001000;
}

void latchHighAddress(){
  PORTC = (PORTC & 0x03) | B001100;
  PORTC = (PORTC & 0x03) | B101100;
  PORTC = (PORTC & 0x03) | B001100;
}

void handover(){
  PORTC = (PORTC & 0x03) | B010100;
}

void enableRAM(){
  PORTC = (PORTC & 0x03) | B000000;
  PORTC = (PORTC & 0x03) | B100000;
  PORTC = (PORTC & 0x03) | B000000;
  PORTC = (PORTC & 0x03) | B100000;
}

void assertWrite(){
  PORTC = (PORTC & 0x111111) | B000011;
  PORTC = (PORTC & 0x111110) | B000010;
}

File readFileFromSDCard(char* fileName) {
  return SD.open("/BINARY.ROM");
}