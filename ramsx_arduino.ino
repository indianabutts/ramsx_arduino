#include <SdFat.h>
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
#define LINE_BUF_SIZE 128  //Maximum input string length
#define ARG_BUF_SIZE 64    //Maximum argument string length
#define MAX_NUM_ARGS 8     //Maximum number of arguments

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

char line[LINE_BUF_SIZE];
char args[MAX_NUM_ARGS][ARG_BUF_SIZE];
char* directory;
char* fileName;

int cmd_list();
int cmd_writeFile();
int cmd_handover();
int cmd_exit();
bool error_flag = false;
//List of functions pointers corresponding to each command
int (*commands_func[])(){
  &cmd_list,
  &cmd_writeFile,
  &cmd_handover,
  &cmd_exit
};

//List of command names
const char* commands_str[] = {
  "list",
  "write",
  "handover",
  "exit"
};

//List of LED sub command names
const char* write_args[] = {
  "swap",
  "with_handover",
};

int num_commands = sizeof(commands_str) / sizeof(char*);

sd_t sd;


void cli_init() {
  Serial.println("\nWelcome to the RAMSX Serial Monitor");
  Serial.println("Type \"help\" to see a list of commands");
}

void ramsx_cli() {
  Serial.print("> ");
  read_line();
  if (!error_flag) {
    parse_line();
  }
  if (!error_flag) {
    execute();
  }

  memset(line, 0, LINE_BUF_SIZE);
  memset(args, 0, sizeof(args[0][0]) * MAX_NUM_ARGS * ARG_BUF_SIZE);

  error_flag = false;
}

void read_line() {
  String line_string;

  while (!Serial.available())
    ;

  if (Serial.available()) {
    line_string = Serial.readStringUntil("\n");
    if (line_string.length() < LINE_BUF_SIZE) {
      line_string.toCharArray(line, LINE_BUF_SIZE);
      Serial.println(line_string);
    } else {
      Serial.println("Input string too long");
      error_flag = true;
    }
  }
}

void parse_line() {
  char *argument;
  unsigned int counter = 0;

  argument = strtok(line,":");
  while((argument != NULL)){
    if(counter<MAX_NUM_ARGS){
      if(strlen(argument)<ARG_BUF_SIZE){
        strcpy(args[counter], argument);
        argument = strtok(NULL, ":");
        counter++;
      }
      else {
        Serial.println("Input String too long");
        error_flag = true;
        break;
      }
    }
    else {
      break;
    }
  }
}

int execute(){
  for(int i = 0; i<num_commands; i++){
    if(strcmp(args[0], commands_str[i])==0){
      return(*commands_func[i])();
    }
  }

  Serial.println("Invalid Command. Type \"help\" for more ");
  return 0;
}

void setupPinsForWrite() {
  DDRD = DDRD | B11111100;
  DDRB = DDRB | B00000011;
  DDRC = DDRC | B111111;
  Serial.println("\nSetting Port D2-7, B0-1, C0-5 as Outputs");
}
void setDataPinValue(byte data) {
  // Serial.println(data);
  // Put lower 6 bits onto port D2-7
  // Put upper 8 bits onto port B0-1
  PORTD = (((data << 2) & 0xFC)) | PORTD & 0x3;
  PORTB = (data >> 6) & 0x3 | PORTB & 0xFC;
}

int cmd_help(){
  Serial.println("HELP COMMAND");
  return 1;
}

int cmd_list(){
  Serial.println("LIST COMMAND");
  return 1;
}

int cmd_writeFile(){
  Serial.println("Write File");
  return 1;
}

int cmd_exit(){
  Serial.println("Exit");
  return 1;
}

int cmd_handover(){
  Serial.println("Handover");
  return 1;
}

int writeFileToSRAM(const char* fileName) {
  setupPinsForWrite();
  file_t romFile = sd.open(fileName);
  uint32_t romSize = (uint32_t)romFile.fileSize();

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

  Serial.print("\nStarting ROM Read with Offset ");
  Serial.print(offset, HEX);
  Serial.print(" and size ");
  Serial.print(romSize);
  Serial.print("kb");
  long int startTime = millis();
  unsigned int address = 0;
  assertReset();
  assertWrite();

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
  Serial.print("\nHanding over to MSX, use signal ");
  Serial.print(chipSelect);
  handover();
  while (true)
    ;
}

void displayDirectoryContent(sd_t sd, file_t& aDirectory, byte tabulation) {
  file_t file;
  char fileName[255];

  if (!aDirectory.isDir()) return;
  aDirectory.rewind();
  unsigned int counter = 0;
  file.getName(fileName, sizeof(fileName));
  Serial.print("\n\n Listing for DIR: ");
  Serial.print(fileName);
  Serial.print("\n=================================================\n");
  Serial.print("File Name                    \t Size \t Start Address\n");
  Serial.print("=================================================\n");
  while (file.openNext(&aDirectory, O_READ)) {
    if (!file.isHidden()) {
      file.getName(fileName, sizeof(fileName));
      fileName[30] = '\0';
      for (uint8_t i = 0; i < tabulation; i++) Serial.write('\t');
      if (file.isDir()) {
        Serial.println(F("/"));
        Serial.print(fileName);
      } else {
        file.seek(3);
        byte msb = file.peek();
        file.seek(2);
        byte lsb = file.peek();
        Serial.print(fileName);
        Serial.write('\t');
        Serial.print((uint32_t)file.fileSize() / 1000);
        Serial.print(" kb \t");
        Serial.print("0x");
        Serial.print(msb, HEX), Serial.print(lsb, HEX);
        Serial.print("\n");
      }
    }
    counter++;
    file.close();
  }
  Serial.print("\n ");
  Serial.print(counter);
  Serial.print(" total files in directory");
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
  PORTC = (PORTC & B000000) | B000010;
}

void assertReset() {
  PORTC = (PORTC & 0x03) | B000100;
  PORTC = (PORTC & 0x03) | B100100;
}

void setup() {
  Serial.begin(115200);
  Serial.print("\n******* Starting New Run *******\n");
  while (!Serial)
    ;
  sd = initializeSDCard(10);
  Serial.print("\nSD Card Initialized");
  cli_init();
}

void loop() {
  ramsx_cli();
}


// File readFileFromSDCard(char* fileName) {
//   return SD.open(fileName);
// }