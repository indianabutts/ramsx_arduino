#ifndef SD_UTILITIES_H
#define SD_UTILITIES_H
#include <SdFat.h>

#define SD_FAT_TYPE 2
#define SD_INDEX_FILE_NAME "._index"
#define SD_DEFAULT_FILE_NAME_SIZE 20

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
  char* fileName;
} SD_RomFile;

sd_t sd_initializeSDCard(int pin);
void sd_createIndexFile(sd_t sd, file_t& directory, char* filename);
void sd_getNFilesFromOffset(sd_t sd, file_t& directory, uint8_t fileCount, uint16_t pageNumber);
void sd_displayDirectoryContent(sd_t sd, file_t& aDirectory, byte tabulation);

#endif