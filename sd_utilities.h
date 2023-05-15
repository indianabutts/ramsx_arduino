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
  uint16_t fileSize;
  uint16_t offset;
  char fileName[101];
} SD_RomFile;

sd_t sd_initializeSDCard(int pin);
uint16_t sd_totalFilesInDirectory(file_t& directory);
void sd_buildIndexFile(sd_t sd, file_t& directory, file_t& indexFile);
void sd_seekToFileOffset(sd_t& sd, file_t& directory, uint8_t fileCount, uint16_t pageNumber);
SD_RomFile* sd_getNFilenamesFromOffset(file_t& indexFile, uint8_t pageNumber, uint8_t count);
SD_RomFile sd_getFileFromOffset(sd_t& sd, file_t& directory);
void sd_displayDirectoryContent(sd_t sd, file_t& aDirectory, byte tabulation);
SD_RomFile sd_getFileInfo(file_t& file);
#endif