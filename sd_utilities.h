#ifndef SD_UTILITIES_H
#define SD_UTILITIES_H
#include <SdFat.h>
#define SD_FAT_TYPE 2
#define SD_INDEX_FILE_NAME "._index"
#define SD_DEFAULT_FILE_NAME_SIZE 21
#define SD_DEFAULT_FILE_COUNT 21
#define SD_DEFAULT_FILE_SIZE 5

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

typedef struct RomFile{
  unsigned int fileSize;
  uint16_t offset;
  char fileName[SD_DEFAULT_FILE_NAME_SIZE];
  char formattedFileSize[5];
} SD_RomFile;

sd_t sd_initializeSDCard(int pin);

void sd_createIndexFile(sd_t sd, file_t& directory, char* filename);
void sd_seekToFileOffset(sd_t& sd, file_t& directory, uint8_t fileCount, uint16_t pageNumber);
SD_RomFile sd_getFileFromOffset(sd_t& sd, file_t& directory);
void sd_displayDirectoryContent(sd_t& sd, file_t& aDirectory, byte tabulation);
SD_RomFile sd_getFileInfo(file_t& file);
#endif