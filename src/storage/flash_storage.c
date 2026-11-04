#include "storage/flash_storage.h"

#include "drivers/nvmctrl.h"

#define FLASH_VALID_RECORD 0x46535652 // FSVR

// Macro for finding storage type settings
#define FLASH_REQUIRE_NEWEST_LIST(X, val)                                      \
  X(FLASH_STORAGE_TYPE_SETTINGS, val)                                          \
  X(FLASH_STORAGE_TYPE_IMAGE, val)

#define GENERATE_OR_CHAIN(item, val) (val) == (item) ||

#define IS_IN_FLASH_REQUIRE_NEWEST(val, list_macro)                            \
  (list_macro(GENERATE_OR_CHAIN,                                               \
              val) 0) // initalize flash storage version settings

FlashVersionSetting flashVersionSettings = {
    .Flash_Storage_Settings_Versions = FLASH_STORAGE_SETTINGS_VERSION,
    .Flash_Storage_Game_History_Versions = FLASH_STORAGE_GAME_HISTORY_VERSION,
    .Flash_Storage_Image_Versions = FLASH_STORAGE_IMAGE_VERSION};

static uint32_t flash_crc32_update(uint32_t crc, const uint8_t *data,
                                   uint32_t length) {
  for (uint32_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint32_t bit = 0; bit < 8; bit++) {
      crc = (crc >> 1) ^ ((crc & 1U) ? 0xEDB88320U : 0U);
    }
  }
  return crc;
}

static bool flash_calculate_record_crc(const FlashRecordHeader *record,
                                       const uint8_t *payload, uint32_t length,
                                       uint32_t *checksum) {
  if (record == NULL || payload == NULL || checksum == NULL ||
      record->payloadLength == 0 || length != record->payloadLength) {
    return false;
  }

  FlashRecordHeader header = *record;
  header.checksum = 0;

  uint32_t crc =
      flash_crc32_update(0xFFFFFFFFU, (const uint8_t *)&header, sizeof(header));
  crc = flash_crc32_update(crc, payload, length);
  *checksum = crc ^ 0xFFFFFFFFU;
  return true;
}

bool flash_check_check_sum(const FlashRecordHeader *record, const uint8_t *data,
                           uint32_t length) {

  if (record == NULL || data == NULL || record->payloadLength == 0) {
    return false;
  }

  uint32_t headerSize = sizeof(FlashRecordHeader);
  uint32_t totalSize = headerSize + record->payloadLength;

  const uint8_t *payloadPtr = data;

  if (length == totalSize) {
    payloadPtr += headerSize;
  } else if (length != record->payloadLength) {
    return false;
  }

  uint32_t checksum = 0;
  return flash_calculate_record_crc(record, payloadPtr, record->payloadLength,
                                    &checksum) &&
         checksum == record->checksum;
}

uintptr_t flash_find_next_page(void) {
  // determine how many pages are in the reserved user space
  uint32_t numPages = NVM_STORAGE_SIZE / FLASH_PAGE_SIZE;

  // create the page pointer
  const uint8_t *pagePtr = (const uint8_t *)(uintptr_t)NVM_STORAGE_START;

  // loop through each page
  bool hasData;

  for (uint32_t pageIndex = 0; pageIndex < numPages; pageIndex++) {
    hasData = false;
    // loop through all bytes in each page to check if erased
    for (uint32_t byteIndex = 0; byteIndex < FLASH_PAGE_SIZE; byteIndex++) {
      if (pagePtr[(pageIndex * FLASH_PAGE_SIZE) + byteIndex] != 0xFF) {
        // begin searching next page
        hasData = true;
        break;
      }
    }

    if (!hasData) {
      // empty page found return the address
      return (uintptr_t)&pagePtr[(pageIndex * FLASH_PAGE_SIZE)];
    }
  }

  // no empty pages found
  return 0U;
}

uintptr_t flash_find_free_pages(uint32_t numPages, uint32_t blockIndex) {
  // get total number of reserved pages
  uint32_t totalUserPages = NVM_BLOCK_SIZE / FLASH_PAGE_SIZE;

  uint32_t numBlocks = NVM_STORAGE_SIZE / NVM_BLOCK_SIZE;

  // find the first available page
  uintptr_t startingAddress = 0;

  // make sure there is space in the defined region
  if (numPages > totalUserPages || numPages == 0 || blockIndex >= numBlocks) {
    return 0;
  }

  // find which page to start at index wize
  uint32_t startingAddressIndex = 0;

  // create flag
  uint8_t consecutivePages = 0;

  // create flash pointer
  const uint8_t *flashPtr =
      (const uint8_t *)(uintptr_t)(NVM_STORAGE_START +
                                   (NVM_BLOCK_SIZE * blockIndex));

  // create page flag
  bool hasData = false;

  // loop through each page
  for (; startingAddressIndex < totalUserPages; startingAddressIndex++) {

    // reset has data
    hasData = false;

    // loop through each byte
    for (uint32_t byteIndex = 0; byteIndex < FLASH_PAGE_SIZE; byteIndex++) {
      if (flashPtr[(startingAddressIndex * FLASH_PAGE_SIZE) + byteIndex] !=
          0xFF) {
        consecutivePages = 0;
        hasData = true;
        break;
      }
    }

    // if first page and no data store address, if not first page keep address
    startingAddress =
        (consecutivePages == 0)
            ? ((hasData) ? 0
                         : (uintptr_t)&flashPtr[(startingAddressIndex *
                                                 FLASH_PAGE_SIZE)])
            : startingAddress;

    // if page not empty restart count
    consecutivePages = hasData ? 0 : consecutivePages + 1;

    // check if num of pages met
    if (consecutivePages == numPages) {
      return startingAddress;
    }
  }

  return 0;
}

uint32_t flash_find_required_num_pages(FlashRecordHeader *record) {
  // make sure the record exists
  if (record == NULL)
    return 0;

  // make sure it's a valid record
  if (record->magic != FLASH_VALID_RECORD || record->version == 0 ||
      record->sequenceNumber == 0) {
    return 0;
  }

  // get the payload size
  uint32_t payloadlength = record->payloadLength;

  // check if zero
  if (payloadlength == 0) {
    return 0;
  }

  // calculate the total size of the record
  uint32_t totalsize = sizeof(FlashRecordHeader) + payloadlength;

  if (totalsize > NVM_BLOCK_SIZE) {
    return 0;
  }

  // return number of pages required forcing round up
  return ((totalsize + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE);
}

bool flash_write_record(const uint8_t *payloadAddress,
                        const uint32_t payloadLength,
                        const RecordType recordType) {
  // make sure inputs are valid
  if (payloadAddress == NULL || payloadLength == 0 ||
      payloadLength > NVM_BLOCK_SIZE - sizeof(FlashRecordHeader) ||
      !flash_check_valid_version_num(recordType,
                                     flash_get_version_num(recordType))) {
    return false;
  }

  // calculate total size
  uint32_t totalSize = payloadLength + sizeof(FlashRecordHeader);

  // find the number of pages required rounding up
  uint32_t numPages = (totalSize + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

  uint32_t startingAddress = 0;

  // find a vlaid satarting address
  for (uint32_t blockIndex = 0; blockIndex < NVM_STORAGE_SIZE / NVM_BLOCK_SIZE;
       blockIndex++) {

    // find the starting address of the free pages
    startingAddress = flash_find_free_pages(numPages, blockIndex);

    // check if valid
    if (startingAddress == 0 ||
        (startingAddress + (numPages * FLASH_PAGE_SIZE)) >
            (NVM_STORAGE_START + ((blockIndex + 1) * NVM_BLOCK_SIZE))) {
      startingAddress = 0;
      continue;
    } else {
      break;
    }
  }

  if (startingAddress == 0) {
    // no valid starting address found
    return false;
  }

  // typedef struct {
  //   uint32_t magic;
  //   uint32_t sequenceNumber;
  //   uint32_t checksum;
  //   uint16_t payloadLength;
  //   uint8_t version;
  //   RecordType storageType;
  // } FlashRecordHeader;

  // find the sequence number
  uint32_t sequenceNumber = 0;

  // record existingRecordAddress
  uintptr_t existingRecordAddress =
      flash_find_record(recordType, &sequenceNumber);

  if (sequenceNumber == UINT32_MAX)
    return false;

  sequenceNumber++;

  // get version number
  uint32_t versionNum = flash_get_version_num(recordType);

  // create header
  FlashRecordHeader record = {.magic = FLASH_VALID_RECORD,
                              .payloadLength = payloadLength,
                              .storageType = recordType,
                              .version = versionNum,
                              .sequenceNumber = sequenceNumber,
                              .checksum = 0};

  // calculate the required buffSize
  uint32_t buffSize = sizeof(FlashRecordHeader) + payloadLength;

  // create buffer aligned to flash page size
  uint8_t buff[FLASH_PAGE_SIZE * numPages];

  // cast flash header to 1 byte pointer for easier manipulation

  const uint8_t *headerPtr = (uint8_t *)&record;

  // populate the buffer with 0xFF or Data
  for (uint32_t bufferIndex = 0; bufferIndex < FLASH_PAGE_SIZE * numPages;
       bufferIndex++) {
    // if in the header range put the header data
    if (bufferIndex < sizeof(FlashRecordHeader)) {
      buff[bufferIndex] = headerPtr[bufferIndex];
      // if in the payload range headerindex < index < payloadlength
    } else if (bufferIndex >= sizeof(FlashRecordHeader) &&
               bufferIndex < buffSize) {
      buff[bufferIndex] =
          payloadAddress[bufferIndex - sizeof(FlashRecordHeader)];
    } else // if payloadlength < index < bufferSize (adding padding)
    {
      buff[bufferIndex] = 0xFF;
    }
  }

  // calculate checksum
  uint32_t checkSum = 0;
  if (!flash_calculate_record_crc(&record, payloadAddress, payloadLength,
                                  &checkSum)) {
    return false;
  }

  // 8 bytes into the header is the location of checksum
  uint32_t checkSumOffset = sizeof(uint32_t) + sizeof(uint32_t);

  // write the new value of check sum
  const uint8_t *checkSumPtr = (const uint8_t *)&checkSum;
  for (uint32_t i = 0; i < sizeof(checkSum); i++) {
    buff[checkSumOffset + i] = checkSumPtr[i];
  }

  // write the buffer to flash making sure to resepct page boundries
  // Program continuation pages first
  for (uint32_t pageIndex = 1; pageIndex < numPages; pageIndex++) {

    uint32_t offset = FLASH_PAGE_SIZE * pageIndex;
    uint32_t pageAddress = startingAddress + offset;

    if (!nvmctrl_write_page(pageAddress, &buff[offset], FLASH_PAGE_SIZE)) {
      return false;
    }
  }

  // Program the header-bearing page last
  if (!nvmctrl_write_page(startingAddress, buff, FLASH_PAGE_SIZE)) {
    return false;
  }

  return true;
}

bool test_flash_write_record(const uint8_t *payloadAddress,
                             const uint32_t payloadLength,
                             const RecordType recordType) {
  // make sure inputs are valid
  if (payloadAddress == NULL || payloadLength == 0 ||
      payloadLength > NVM_BLOCK_SIZE - sizeof(FlashRecordHeader) ||
      !flash_check_valid_version_num(recordType,
                                     flash_get_version_num(recordType))) {
    return false;
  }

  // calculate total size
  uint32_t totalSize = payloadLength + sizeof(FlashRecordHeader);

  // find the number of pages required rounding up
  uint32_t numPages = (totalSize + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

  // find the starting address of the free pages
  uint32_t startingAddress = 0;

  // find a vlaid satarting address
  for (uint32_t blockIndex = 0; blockIndex < NVM_STORAGE_SIZE / NVM_BLOCK_SIZE;
       blockIndex++) {

    // find the starting address of the free pages
    startingAddress = flash_find_free_pages(numPages, blockIndex);

    // check if valid
    if (startingAddress == 0 ||
        (startingAddress + (numPages * FLASH_PAGE_SIZE)) >
            (NVM_STORAGE_START + ((blockIndex + 1) * NVM_BLOCK_SIZE))) {
      startingAddress = 0;
      continue;
    } else {
      break;
    }
  }

  if (startingAddress == 0) {
    // no valid starting address found
    return false;
  }

  // typedef struct {
  //   uint32_t magic;
  //   uint32_t sequenceNumber;
  //   uint32_t checksum;
  //   uint16_t payloadLength;
  //   uint8_t version;
  //   RecordType storageType;
  // } FlashRecordHeader;

  // find the sequence number
  uint32_t sequenceNumber = 0;

  // record existingRecordAddress
  uintptr_t existingRecordAddress =
      flash_find_record(recordType, &sequenceNumber);

  if (sequenceNumber == UINT32_MAX)
    return false;

  sequenceNumber++;

  // get version number
  uint32_t versionNum = flash_get_version_num(recordType);

  // create header
  FlashRecordHeader record = {.magic = FLASH_VALID_RECORD,
                              .payloadLength = payloadLength,
                              .storageType = recordType,
                              .version = versionNum,
                              .sequenceNumber = sequenceNumber,
                              .checksum = 0};

  // calculate checksum
  if (!flash_calculate_record_crc(&record, payloadAddress, payloadLength,
                                  &record.checksum)) {
    return false;
  }

  // calculate the required buffSize
  uint32_t buffSize = sizeof(FlashRecordHeader) + payloadLength;

  // create buffer aligned to flash page size
  uint8_t buff[FLASH_PAGE_SIZE * numPages];

  // cast flash header to 1 byte pointer for easier manipulation

  const uint8_t *headerPtr = (uint8_t *)&record;

  // populate the buffer with 0xFF or Data
  for (uint32_t bufferIndex = 0; bufferIndex < FLASH_PAGE_SIZE * numPages;
       bufferIndex++) {
    // if in the header range put the header data
    if (bufferIndex < sizeof(FlashRecordHeader)) {
      buff[bufferIndex] = headerPtr[bufferIndex];
      // if in the payload range headerindex < index < payloadlength
    } else if (bufferIndex >= sizeof(FlashRecordHeader) &&
               bufferIndex < buffSize) {
      buff[bufferIndex] =
          payloadAddress[bufferIndex - sizeof(FlashRecordHeader)];
    } else // if payloadlength < index < bufferSize (adding padding)
    {
      buff[bufferIndex] = 0xFF;
    }
  }

  // write the buffer to flash making sure to resepct page boundries
  // Program continuation pages first
  for (uint32_t pageIndex = 1; pageIndex < numPages; pageIndex++) {

    uint32_t offset = FLASH_PAGE_SIZE * pageIndex;
    uint32_t pageAddress = startingAddress + offset;

    if (!nvmctrl_write_page(pageAddress, &buff[offset], FLASH_PAGE_SIZE)) {
      return false;
    }
  }

  // // Program the header-bearing page last
  // if (!nvmctrl_write_page(startingAddress, buff, FLASH_PAGE_SIZE)) {
  //   return false;
  // }

  return true;
}

bool flash_read_record(const uint32_t recordAddress, uint8_t *buffer,
                       uint32_t buffCap, uint32_t *bytesRead) {

  // validate the user provide buffer
  if (buffer == NULL || buffCap == 0 || bytesRead == NULL) {
    return false;
  }
  *bytesRead = 0;

  // validate the flash address
  if (recordAddress < NVM_STORAGE_START || recordAddress >= NVM_STORAGE_END ||
      recordAddress % FLASH_PAGE_SIZE) {
    return false;
  }

  // read the header information
  FlashRecordHeader header;

  uint8_t *headerPtr = (uint8_t *)&header;

  if (!nvmctrl_read_flash(recordAddress, headerPtr, sizeof(header))) {
    return false;
  }

  // check if valid record
  if (header.magic != FLASH_VALID_RECORD ||
      !flash_check_valid_version_num(header.storageType, header.version) ||
      header.payloadLength == 0) {
    return false;
  }

  // find the number of pages to read
  uint32_t numPages = flash_find_required_num_pages(&header);

  uint32_t blockEnd =
      NVM_STORAGE_START +
      (((recordAddress - NVM_STORAGE_START) / NVM_BLOCK_SIZE) + 1) *
          NVM_BLOCK_SIZE;
  uint32_t availablePages = (blockEnd - recordAddress) / FLASH_PAGE_SIZE;

  if (numPages == 0 || numPages > availablePages) {
    return false;
  }

  // calculate total size of the record
  uint32_t totalsize = (sizeof(header) + header.payloadLength);

  // boundry check
  if (totalsize > blockEnd - recordAddress) {
    return false;
  }

  // ensure the user's buffer has enough capacitiy
  if (totalsize > buffCap) {
    return false;
  }

  // read the flash memmory directly into the users buffer
  if (!nvmctrl_read_flash(recordAddress, buffer, totalsize)) {
    return false;
  };

  *bytesRead = totalsize;

  // return the checksum check
  return flash_check_check_sum(&header, buffer, totalsize);
}

bool flash_read_record_header(const uint32_t recordAddress,
                              FlashRecordHeader *header) {
  // validate the flash address
  if (recordAddress < NVM_STORAGE_START || recordAddress >= NVM_STORAGE_END ||
      recordAddress % FLASH_PAGE_SIZE) {
    return false;
  }

  if (header == NULL) {
    return false;
  }

  uint8_t *headerPtr = (uint8_t *)header;

  if (!nvmctrl_read_flash(recordAddress, headerPtr, sizeof(*header))) {
    return false;
  }

  return true;
}

bool flash_read_latest_record(RecordType recordType, uint8_t *buffer,
                              uint32_t buffCap, uint32_t *bytesRead) {
  // check inputs
  if (buffer == NULL || bytesRead == NULL || buffCap == 0) {
    return false;
  }

  // reset bytes read for error checking
  *bytesRead = 0;

  uint32_t sequenceNum = 0;

  // find the newest record
  const uintptr_t recordAddress = flash_find_record(recordType, &sequenceNum);

  // check if an address exists
  if (recordAddress == 0) {
    return false;
  }

  // attempt to read the record also validates checksum too in the function
  return flash_read_record(recordAddress, buffer, buffCap, bytesRead);
}

uintptr_t flash_find_record(RecordType recordType, uint32_t *sequenceNumber) {
  if (sequenceNumber == NULL) {
    return 0;
  }
  *sequenceNumber = 0;
  if (!flash_check_valid_version_num(recordType,
                                     flash_get_version_num(recordType))) {
    return 0;
  }

  // calculate how many user pages exist
  uint32_t totalUserPages = NVM_STORAGE_SIZE / FLASH_PAGE_SIZE;

  // define starting address
  uint32_t currentAddress = NVM_STORAGE_START;

  // define search header
  FlashRecordHeader searchHeader;

  // define currently best sequence number
  uint32_t sequenceNum = 0U;

  // define record address
  uintptr_t recordAddress = 0U;

  // loop through each page
  uint32_t currentPage = 0;

  while (currentPage < totalUserPages) {
    // set the current address
    currentAddress = NVM_STORAGE_START + (currentPage * FLASH_PAGE_SIZE);

    // read the header
    if (!flash_read_record_header(currentAddress, &searchHeader)) {
      // advance to next page
      currentPage++;
      continue;
    }

    // check if valid record
    if (searchHeader.magic != FLASH_VALID_RECORD ||
        !flash_check_valid_version_num(searchHeader.storageType,
                                       searchHeader.version) ||
        searchHeader.payloadLength == 0) {
      // advance to next page
      currentPage++;
      continue;
    }

    // find the number of pages for the record
    uint32_t numPages = flash_find_required_num_pages(&searchHeader);

    // verify the number of pages
    uint32_t blockEnd =
        NVM_STORAGE_START +
        (((currentAddress - NVM_STORAGE_START) / NVM_BLOCK_SIZE) + 1) *
            NVM_BLOCK_SIZE;
    if (numPages == 0 ||
        numPages > (blockEnd - currentAddress) / FLASH_PAGE_SIZE) {
      // advance to next page
      currentPage++;
      continue;
    }

    const uint8_t *payload =
        (const uint8_t *)(uintptr_t)(currentAddress +
                                     sizeof(FlashRecordHeader));

    if (!flash_check_check_sum(&searchHeader, payload,
                               searchHeader.payloadLength)) {
      currentPage++;
      continue;
    }

    // check if record matches type
    if (searchHeader.storageType == recordType) {
      // check if this record is newer
      if (searchHeader.sequenceNumber > sequenceNum) {
        // set return to this header
        sequenceNum = searchHeader.sequenceNumber;
        recordAddress = currentAddress;
      }
    }

    // advance by number of pages
    currentPage += numPages;
  }

  // return sequence number since we have already found it
  *sequenceNumber = sequenceNum;

  return recordAddress;
}

uint32_t flash_get_version_num(RecordType recordType) {
  switch (recordType) {
  case FLASH_STORAGE_TYPE_SETTINGS: {
    return flashVersionSettings.Flash_Storage_Settings_Versions;
  }
  case FLASH_STORAGE_TYPE_GAME_HISTORY: {
    return flashVersionSettings.Flash_Storage_Game_History_Versions;
  }
  case FLASH_STORAGE_TYPE_IMAGE: {
    return flashVersionSettings.Flash_Storage_Image_Versions;
  }
  default: {
    return 0;
  }
  }
}

bool flash_check_valid_version_num(RecordType recordType, uint32_t versionNum) {
  if (versionNum == 0) {
    return false;
  }

  switch (recordType) {
  case FLASH_STORAGE_TYPE_SETTINGS: {
    return (versionNum <= flashVersionSettings.Flash_Storage_Settings_Versions);
  }
  case FLASH_STORAGE_TYPE_GAME_HISTORY: {
    return (versionNum <=
            flashVersionSettings.Flash_Storage_Game_History_Versions);
  }
  case FLASH_STORAGE_TYPE_IMAGE: {
    return (versionNum <= flashVersionSettings.Flash_Storage_Image_Versions);
  }
  default: {
    return false;
  }
  }
}

bool flash_storage_analyze(FlashRecord *flashRecordPtr, uint32_t capacity,
                           uint32_t *len, uint32_t blockAddress) {

  if (flashRecordPtr == NULL || len == NULL) {
    return false;
  }
  *len = 0;
  if (blockAddress < NVM_STORAGE_START || blockAddress >= NVM_STORAGE_END ||
      (blockAddress - NVM_STORAGE_START) % NVM_BLOCK_SIZE != 0 ||
      NVM_STORAGE_END - blockAddress < NVM_BLOCK_SIZE) {
    return false;
  }
  // create base address
  uint32_t base = blockAddress;

  // calculate the total number of user pages
  uint32_t totalPages = NVM_BLOCK_SIZE / FLASH_PAGE_SIZE;

  if (capacity < totalPages) {
    return false;
  }

  // reset length
  *len = 0;

  // track record information
  uint32_t recordIndex = 0;
  FlashState recordState;
  uint32_t recordAddress;

  // define sequence number array
  uint32_t sequenceNums[NUM_FLASH_STORAGE_TYPES];

  // define newest index array
  uint32_t newIndexLoc[NUM_FLASH_STORAGE_TYPES];

  // define old num pages
  uint32_t oldNumPages[NUM_FLASH_STORAGE_TYPES];

  bool multiSequcence[NUM_FLASH_STORAGE_TYPES];

  // itialize the arrays
  for (uint32_t i = 0; i < NUM_FLASH_STORAGE_TYPES; i++) {
    sequenceNums[i] = 0;
    oldNumPages[i] = 0;
    multiSequcence[i] = false;
    newIndexLoc[i] = UINT32_MAX;
  }

  // loop through each page looking for headers
  for (uint32_t pageIndex = 0; pageIndex < totalPages; pageIndex++) {
    // calculate the current address
    uint32_t currentPageAddress = base + (FLASH_PAGE_SIZE * pageIndex);

    // create header object
    FlashRecordHeader pageHeader;

    // populate the header object
    if (!flash_read_record_header(currentPageAddress, &pageHeader)) {
      // ERROR
      return false;
    }

    // validate the header
    if (pageHeader.magic != FLASH_VALID_RECORD ||
        !flash_check_valid_version_num(pageHeader.storageType,
                                       pageHeader.version) ||
        pageHeader.sequenceNumber == 0) {
      // set the record address
      recordAddress = currentPageAddress;

      // check for erased data
      recordState = FLASH_STATE_ERASED;

      // create a pointer for easy searching
      const uint8_t *flashPtr = (const uint8_t *)(uintptr_t)recordAddress;

      for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        if (flashPtr[i] != 0xFF) {
          recordState = FLASH_STATE_INVALID;
        }
      }

      // create record
      FlashRecord record = {.flashRecordAddress = recordAddress,
                            .flashRecordSize = FLASH_PAGE_SIZE,
                            .flashRecordState = recordState};

      // store the record information
      flashRecordPtr[recordIndex] = record;

      // increment the record index
      recordIndex++;
      // continue searching for more records
      continue;
    }

    // set recordAddress to this page address
    recordAddress = currentPageAddress;

    uint32_t totalRecordSize =
        pageHeader.payloadLength + sizeof(FlashRecordHeader);

    // check record does not span multiple blocks
    if (pageHeader.payloadLength == 0 || totalRecordSize > NVM_BLOCK_SIZE ||
        totalRecordSize > ((base + NVM_BLOCK_SIZE) - currentPageAddress)) {
      // create record
      FlashRecord record = {.flashRecordAddress = recordAddress,
                            .flashRecordSize = FLASH_PAGE_SIZE,
                            .flashRecordState = FLASH_STATE_INVALID};

      // store the record information
      flashRecordPtr[recordIndex] = record;

      // increment the record index
      recordIndex++;
      // continue searching for more records
      continue;
    }

    // checksum check
    if (!flash_check_check_sum(
            &pageHeader,
            (const uint8_t *)(uintptr_t)(recordAddress +
                                         sizeof(FlashRecordHeader)),
            pageHeader.payloadLength)) {
      // create record
      FlashRecord record = {.flashRecordAddress = recordAddress,
                            .flashRecordSize = FLASH_PAGE_SIZE,
                            .flashRecordState = FLASH_STATE_INVALID};
      // store record
      flashRecordPtr[recordIndex] = record;

      // increment the counter
      recordIndex++;

      continue;
    }

    // define defualt state
    FlashState recordState = FLASH_STATE_LIVE;

    // define how many pages to skip
    uint32_t numPagesSkip =
        (totalRecordSize + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    // sequence number checking
    // newer record
    if (sequenceNums[pageHeader.storageType] < pageHeader.sequenceNumber) {
      if (IS_IN_FLASH_REQUIRE_NEWEST(pageHeader.storageType,
                                     FLASH_REQUIRE_NEWEST_LIST)) {
        if (newIndexLoc[pageHeader.storageType] == UINT32_MAX) {
          // if this is the first newest set it to this page
          sequenceNums[pageHeader.storageType] = pageHeader.sequenceNumber;
          newIndexLoc[pageHeader.storageType] = pageIndex;
          oldNumPages[pageHeader.storageType] = numPagesSkip;
        } else {

          for (uint32_t i = 0; i < oldNumPages[pageHeader.storageType]; i++) {
            // else set the old record stale and update newest to this index
            flashRecordPtr[newIndexLoc[pageHeader.storageType] + i]
                .flashRecordState = FLASH_STATE_STALE;
          }

          if (multiSequcence[pageHeader.storageType]) {
            for (uint32_t i = 0; i < pageIndex; i++) {
              if (flashRecordPtr[i].flashRecordState != FLASH_STATE_LIVE) {
                continue;
              }
              uint32_t examineAddress = flashRecordPtr[i].flashRecordAddress;
              FlashRecordHeader head;
              if (!flash_read_record_header(examineAddress, &head)) {
                return false;
              }

              if (head.sequenceNumber < pageHeader.sequenceNumber &&
                  head.storageType == pageHeader.storageType) {
                uint32_t j = 0;
                while (1) {
                  if (i + j >= pageIndex) {
                    break;
                  }
                  if (flashRecordPtr[i + j].flashRecordAddress ==
                      examineAddress) {
                    flashRecordPtr[i + j].flashRecordState = FLASH_STATE_STALE;
                  } else {
                    break;
                  }

                  j++;
                }
                i += j - 1;
              }
            }
            multiSequcence[pageHeader.storageType] = false;
          }

          sequenceNums[pageHeader.storageType] = pageHeader.sequenceNumber;
          newIndexLoc[pageHeader.storageType] = pageIndex;
          oldNumPages[pageHeader.storageType] = numPagesSkip;
        }
      }
      // older record
    } else if (sequenceNums[pageHeader.storageType] >
               pageHeader.sequenceNumber) {
      if (IS_IN_FLASH_REQUIRE_NEWEST(pageHeader.storageType,
                                     FLASH_REQUIRE_NEWEST_LIST)) {
        recordState = FLASH_STATE_STALE;
      }
    } else if (IS_IN_FLASH_REQUIRE_NEWEST(pageHeader.storageType,
                                          FLASH_REQUIRE_NEWEST_LIST) &&
               sequenceNums[pageHeader.storageType] ==
                   pageHeader.sequenceNumber) {
      multiSequcence[pageHeader.storageType] = true;
    }

    // define the record
    FlashRecord record = {.flashRecordAddress = recordAddress,
                          .flashRecordSize = FLASH_PAGE_SIZE,
                          .flashRecordState = recordState};

    // set each page to the record
    for (uint32_t i = 0; i < numPagesSkip; i++) {
      flashRecordPtr[recordIndex++] = record;
    }

    // increment the page index while compensating for the loop adding one too
    pageIndex += (numPagesSkip - 1);
  }

  // return the number of records
  *len = recordIndex;

  return true;
}

uintptr_t flash_get_block_address(uint32_t blockIndex) {
  uint32_t numBlocks = NVM_STORAGE_SIZE / NVM_BLOCK_SIZE;

  if (blockIndex >= numBlocks) {
    return UINT32_MAX;
  }

  return NVM_STORAGE_START + (blockIndex * NVM_BLOCK_SIZE);
}

