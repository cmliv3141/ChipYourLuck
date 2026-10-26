#ifndef INC_STORAGE_FLASH_STORAGE
#define INC_STORAGE_FLASH_STORAGE
#include "board.h"

#include <stdbool.h>

// storage types will continue to add more over time
typedef enum : uint8_t {
  FLASH_STORAGE_TYPE_SETTINGS,
  FLASH_STORAGE_TYPE_GAME_HISTORY,
  FLASH_STORAGE_TYPE_IMAGE
} RecordType;

// header struct for flash storage
typedef struct {
  uint32_t magic;
  uint32_t sequenceNumber;
  uint32_t checksum;
  uint16_t payloadLength;
  uint8_t version;
  RecordType storageType;
} FlashRecordHeader;

_Static_assert(sizeof(FlashRecordHeader) == 16,
               "Flash record header must be 16 bytes");

typedef struct {
  uint8_t Flash_Storage_Settings_Versions;
  uint8_t Flash_Storage_Game_History_Versions;
  uint8_t Flash_Storage_Image_Versions;
} FlashVersionSetting;

extern FlashVersionSetting flashVersionSettings;

uintptr_t flash_find_next_page(void);

bool flash_check_check_sum(const FlashRecordHeader *record, const uint8_t *data,
                           uint32_t length);

uint32_t flash_find_required_num_pages(FlashRecordHeader *record);

uintptr_t flash_find_free_pages(uint32_t numPages);

bool flash_write_record(const uint8_t *payloadAddress,
                        const uint32_t payloadLength,
                        const RecordType recordType);

bool flash_read_record(const uint32_t recordAddress, uint8_t *buffer,
                       uint32_t buffCap, uint32_t *bytesRead);

bool flash_read_record_header(const uint32_t recordAddress,
                              FlashRecordHeader *header);

bool flash_read_latest_record(RecordType recordType, uint8_t *buffer,
                              uint32_t buffCap, uint32_t *bytesRead);

uintptr_t flash_find_record(RecordType recordType, uint32_t *sequenceNum);

uint32_t flash_get_version_num(RecordType recordType);

bool flash_check_valid_version_num(RecordType recordType, uint32_t versionNum);
#endif /* INC_STORAGE_FLASH_STORAGE */
