#include "test/test_storage_flash.h"
#include "test/test.h"

#include "drivers/nvmctrl.h"
#include "uart.h"
#include "storage/flash_storage.h"
#include "utils/crc32.h"


#include <string.h>

// Report a test and stop on the first failure.
#define FLASH_TEST_REQUIRE(condition, ...)                                     \
  do {                                                                         \
    bool passed = (condition);                                                 \
    test_report(passed, __VA_ARGS__);                                          \
    if (!passed) {                                                             \
      uart_puts("[FLASH TEST] Aborted\r\n");                                   \
      return;                                                                  \
    }                                                                          \
  } while (0)

void test_flash_storage(void) {

  uart_puts("\r\n========== FLASH STORAGE TEST ==========\r\n");

  const uint32_t base = NVM_STORAGE_START;
  const uint32_t pageSize = FLASH_PAGE_SIZE;
  const uint32_t totalPages = NVM_STORAGE_SIZE / FLASH_PAGE_SIZE;
  const uint32_t headerSize = sizeof(FlashRecordHeader);

  uint32_t bytesRead = 0;
  uint32_t sequenceNum = 0;
  uintptr_t foundAddress = 0;

  FlashRecordHeader header;

  // Test payloads
  uint8_t settingsA[20];
  uint8_t settingsB[17];
  uint8_t settingsC[9];
  uint8_t history[31];
  uint8_t image[700];

  // Large enough for the largest complete test record
  uint8_t readBuffer[sizeof(FlashRecordHeader) + sizeof(image)];

  // One physical flash page
  uint8_t pageBuffer[FLASH_PAGE_SIZE];

  // Initialize predictable test data
  for (uint32_t i = 0; i < sizeof(settingsA); i++)
    settingsA[i] = (uint8_t)(0x10U + i);

  for (uint32_t i = 0; i < sizeof(settingsB); i++)
    settingsB[i] = (uint8_t)(0x40U + i);

  for (uint32_t i = 0; i < sizeof(settingsC); i++)
    settingsC[i] = (uint8_t)(0x80U + i);

  for (uint32_t i = 0; i < sizeof(history); i++)
    history[i] = (uint8_t)(0xA0U + i);

  for (uint32_t i = 0; i < sizeof(image); i++)
    image[i] = (uint8_t)(i & 0xFFU);

  // ============================================================
  // TEST 1: CRC32
  // ============================================================

  uart_puts("\r\n--- CRC32 TESTS ---\r\n");

  const uint8_t crcTest[] = "123456789";

  FLASH_TEST_REQUIRE(crc32_calculate(crcTest, 9) == 0xCBF43926U,
                     "CRC32 standard vector");

  FLASH_TEST_REQUIRE(crc32_calculate(crcTest, 0) == 0U, "CRC32 empty input");

  // ============================================================
  // TEST 2: VERSION MANAGEMENT
  // ============================================================

  uart_puts("\r\n--- VERSION TESTS ---\r\n");

  FLASH_TEST_REQUIRE(flash_get_version_num(FLASH_STORAGE_TYPE_SETTINGS) > 0,
                     "Settings version configured");

  FLASH_TEST_REQUIRE(flash_get_version_num(FLASH_STORAGE_TYPE_GAME_HISTORY) > 0,
                     "History version configured");

  FLASH_TEST_REQUIRE(flash_get_version_num(FLASH_STORAGE_TYPE_IMAGE) > 0,
                     "Image version configured");

  FLASH_TEST_REQUIRE(flash_check_valid_version_num(
                         FLASH_STORAGE_TYPE_SETTINGS,
                         flash_get_version_num(FLASH_STORAGE_TYPE_SETTINGS)),
                     "Current settings version valid");

  FLASH_TEST_REQUIRE(
      !flash_check_valid_version_num(FLASH_STORAGE_TYPE_SETTINGS, 0),
      "Reject version zero");

  FLASH_TEST_REQUIRE(
      !flash_check_valid_version_num(FLASH_STORAGE_TYPE_SETTINGS, 255),
      "Reject unsupported version");

  FLASH_TEST_REQUIRE(flash_get_version_num((RecordType)255) == 0,
                     "Reject unknown record type");

  // ============================================================
  // TEST 3: ERASED STORAGE
  // ============================================================

  uart_puts("\r\n--- ERASE AND PAGE SEARCH ---\r\n");

  FLASH_TEST_REQUIRE(nvmctrl_erase_block(base), "Erase reserved flash block");

  FLASH_TEST_REQUIRE(flash_find_next_page() == base,
                     "First empty page is page 0");

  FLASH_TEST_REQUIRE(flash_find_free_pages(1) == base, "Find one empty page");

  FLASH_TEST_REQUIRE(flash_find_free_pages(totalPages) == base,
                     "Find all 16 consecutive empty pages");

  FLASH_TEST_REQUIRE(flash_find_free_pages(0) == 0,
                     "Reject request for zero pages");

  FLASH_TEST_REQUIRE(flash_find_free_pages(totalPages + 1U) == 0,
                     "Reject request exceeding storage size");

  sequenceNum = 123U;

  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_SETTINGS, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == 0 && sequenceNum == 0,
                     "No records found in erased storage");

  bytesRead = 123U;

  FLASH_TEST_REQUIRE(!flash_read_latest_record(FLASH_STORAGE_TYPE_SETTINGS,
                                               readBuffer, sizeof(readBuffer),
                                               &bytesRead) &&
                         bytesRead == 0,
                     "Latest read fails on empty storage");

  // ============================================================
  // TEST 4: INVALID WRITE ARGUMENTS
  // ============================================================

  uart_puts("\r\n--- WRITE VALIDATION ---\r\n");

  FLASH_TEST_REQUIRE(
      !flash_write_record(NULL, sizeof(settingsA), FLASH_STORAGE_TYPE_SETTINGS),
      "Reject NULL payload");

  FLASH_TEST_REQUIRE(
      !flash_write_record(settingsA, 0, FLASH_STORAGE_TYPE_SETTINGS),
      "Reject zero-length payload");

  // ============================================================
  // TEST 5: WRITE FIRST RECORD
  // ============================================================

  uart_puts("\r\n--- SINGLE PAGE WRITE ---\r\n");

  FLASH_TEST_REQUIRE(flash_write_record(settingsA, sizeof(settingsA),
                                        FLASH_STORAGE_TYPE_SETTINGS),
                     "Write settings record A");

  FLASH_TEST_REQUIRE(flash_find_next_page() == base + pageSize,
                     "Next free page is page 1");

  FLASH_TEST_REQUIRE(flash_read_record_header(base, &header),
                     "Read settings A header");

  FLASH_TEST_REQUIRE(header.sequenceNumber == 1 &&
                         header.storageType == FLASH_STORAGE_TYPE_SETTINGS &&
                         header.payloadLength == sizeof(settingsA),
                     "Settings A header fields");

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&header) == 1,
                     "Settings A requires one page");

  bytesRead = 0;

  FLASH_TEST_REQUIRE(
      flash_read_record(base, readBuffer, sizeof(readBuffer), &bytesRead),
      "Read settings record A");

  FLASH_TEST_REQUIRE(bytesRead == headerSize + sizeof(settingsA),
                     "Settings A returned length");

  FLASH_TEST_REQUIRE(
      memcmp(readBuffer + headerSize, settingsA, sizeof(settingsA)) == 0,
      "Settings A payload matches");

  FLASH_TEST_REQUIRE(
      flash_check_check_sum(&header, settingsA, sizeof(settingsA)),
      "CRC valid for payload-only input");

  FLASH_TEST_REQUIRE(flash_check_check_sum(&header, readBuffer, bytesRead),
                     "CRC valid for complete record");

  // Check unused physical flash bytes
  FLASH_TEST_REQUIRE(nvmctrl_read_flash(base, pageBuffer, sizeof(pageBuffer)),
                     "Read complete physical page");

  bool paddingValid = true;

  for (uint32_t i = headerSize + sizeof(settingsA); i < FLASH_PAGE_SIZE; i++) {
    if (pageBuffer[i] != 0xFFU) {
      paddingValid = false;
      break;
    }
  }

  FLASH_TEST_REQUIRE(paddingValid, "Unused page bytes remain erased");

  // ============================================================
  // TEST 6: PAGE COUNT CALCULATIONS
  // ============================================================

  uart_puts("\r\n--- PAGE COUNT TESTS ---\r\n");

  FlashRecordHeader tempHeader = header;

  tempHeader.payloadLength = FLASH_PAGE_SIZE - headerSize;

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&tempHeader) == 1,
                     "496-byte payload requires one page");

  tempHeader.payloadLength = FLASH_PAGE_SIZE - headerSize + 1U;

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&tempHeader) == 2,
                     "497-byte payload requires two pages");

  tempHeader.payloadLength = 0;

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&tempHeader) == 0,
                     "Reject zero-length record");

  tempHeader = header;
  tempHeader.magic = 0;

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&tempHeader) == 0,
                     "Reject invalid magic number");

  // ============================================================
  // TEST 7: INVALID READ ARGUMENTS
  // ============================================================

  uart_puts("\r\n--- READ VALIDATION ---\r\n");

  FLASH_TEST_REQUIRE(
      !flash_read_record(base + 1U, readBuffer, sizeof(readBuffer), &bytesRead),
      "Reject unaligned record address");

  FLASH_TEST_REQUIRE(!flash_read_record(NVM_STORAGE_END, readBuffer,
                                        sizeof(readBuffer), &bytesRead),
                     "Reject out-of-range record address");

  FLASH_TEST_REQUIRE(!flash_read_record(base, NULL, 64, &bytesRead),
                     "Reject NULL read buffer");

  FLASH_TEST_REQUIRE(!flash_read_record(base, readBuffer,
                                        headerSize + sizeof(settingsA) - 1U,
                                        &bytesRead),
                     "Reject insufficient buffer capacity");

  FLASH_TEST_REQUIRE(!flash_read_record_header(base, NULL),
                     "Reject NULL header destination");

  // ============================================================
  // TEST 8: MULTIPLE RECORD TYPES AND SEQUENCES
  // ============================================================

  uart_puts("\r\n--- RECORD DISCOVERY ---\r\n");

  FLASH_TEST_REQUIRE(flash_write_record(history, sizeof(history),
                                        FLASH_STORAGE_TYPE_GAME_HISTORY),
                     "Write history record");

  FLASH_TEST_REQUIRE(flash_write_record(settingsB, sizeof(settingsB),
                                        FLASH_STORAGE_TYPE_SETTINGS),
                     "Write settings record B");

  // Page 0 = Settings A, sequence 1
  // Page 1 = History, sequence 1
  // Page 2 = Settings B, sequence 2

  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_SETTINGS, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + 2U * pageSize && sequenceNum == 2,
                     "Find newest settings record (sequence 2)");

  foundAddress =
      flash_find_record(FLASH_STORAGE_TYPE_GAME_HISTORY, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + pageSize && sequenceNum == 1,
                     "Find history record (sequence 1)");

  bytesRead = 0;

  FLASH_TEST_REQUIRE(flash_read_latest_record(FLASH_STORAGE_TYPE_SETTINGS,
                                              readBuffer, sizeof(readBuffer),
                                              &bytesRead),
                     "Read latest settings");

  FLASH_TEST_REQUIRE(
      bytesRead == headerSize + sizeof(settingsB) &&
          memcmp(readBuffer + headerSize, settingsB, sizeof(settingsB)) == 0,
      "Latest settings returns B, not A");

  FLASH_TEST_REQUIRE(flash_read_latest_record(FLASH_STORAGE_TYPE_GAME_HISTORY,
                                              readBuffer, sizeof(readBuffer),
                                              &bytesRead),
                     "Read latest history");

  FLASH_TEST_REQUIRE(
      memcmp(readBuffer + headerSize, history, sizeof(history)) == 0,
      "History payload matches");

  // ============================================================
  // TEST 9: MULTI-PAGE RECORD
  // ============================================================

  uart_puts("\r\n--- MULTI-PAGE RECORD ---\r\n");

  FLASH_TEST_REQUIRE(
      flash_write_record(image, sizeof(image), FLASH_STORAGE_TYPE_IMAGE),
      "Write 700-byte image record");

  // Image starts at page 3 and occupies pages 3 and 4

  FLASH_TEST_REQUIRE(flash_read_record_header(base + 3U * pageSize, &header),
                     "Read image header");

  FLASH_TEST_REQUIRE(flash_find_required_num_pages(&header) == 2,
                     "Image spans two pages");

  bytesRead = 0;

  FLASH_TEST_REQUIRE(flash_read_latest_record(FLASH_STORAGE_TYPE_IMAGE,
                                              readBuffer, sizeof(readBuffer),
                                              &bytesRead),
                     "Read multi-page image record");

  FLASH_TEST_REQUIRE(bytesRead == headerSize + sizeof(image),
                     "Multi-page record byte count");

  FLASH_TEST_REQUIRE(memcmp(readBuffer + headerSize, image, sizeof(image)) == 0,
                     "Multi-page image payload matches");

  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_IMAGE, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + 3U * pageSize && sequenceNum == 1,
                     "Find image record starting at page 3");

  FLASH_TEST_REQUIRE(flash_find_next_page() == base + 5U * pageSize,
                     "Multi-page write occupied both pages");

  // ============================================================
  // TEST 10: CORRUPTION DETECTION IN RAM
  // ============================================================

  uart_puts("\r\n--- CRC CORRUPTION TEST ---\r\n");

  // readBuffer currently holds the valid image record

  readBuffer[headerSize] ^= 0x01U;

  FLASH_TEST_REQUIRE(
      !flash_check_check_sum(&header, readBuffer, headerSize + sizeof(image)),
      "CRC detects modified payload byte");

  // Restore the RAM buffer
  readBuffer[headerSize] ^= 0x01U;

  FLASH_TEST_REQUIRE(
      flash_check_check_sum(&header, readBuffer, headerSize + sizeof(image)),
      "CRC passes after restoring byte");

  // ============================================================
  // TEST 11: CORRUPTED RECORD IN FLASH
  // ============================================================

  uart_puts("\r\n--- CORRUPTED FLASH RECORD ---\r\n");

  // Copy the valid Settings B record into RAM.
  FLASH_TEST_REQUIRE(
      nvmctrl_read_flash(base + 2U * pageSize, pageBuffer, sizeof(pageBuffer)),
      "Prepare corrupted test record");

  FlashRecordHeader corruptedHeader;

  memcpy(&corruptedHeader, pageBuffer, sizeof(corruptedHeader));

  // Make this record appear newer than every valid record.
  corruptedHeader.sequenceNumber = 100U;

  memcpy(pageBuffer, &corruptedHeader, sizeof(corruptedHeader));

  // Corrupt one payload byte without updating the CRC.
  pageBuffer[headerSize] ^= 0x01U;

  // Deliberately put the corrupted record in page 6,
  // leaving page 5 empty to test fragmented free space.
  const uint32_t corruptAddress = base + 6U * pageSize;

  FLASH_TEST_REQUIRE(
      nvmctrl_write_page(corruptAddress, pageBuffer, sizeof(pageBuffer)),
      "Program deliberately corrupted record");

  bytesRead = 0;

  FLASH_TEST_REQUIRE(!flash_read_record(corruptAddress, readBuffer,
                                        sizeof(readBuffer), &bytesRead),
                     "Read rejects corrupted flash record");

  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_SETTINGS, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + 2U * pageSize && sequenceNum == 2,
                     "Scanner ignores corrupted sequence 100");

  // ============================================================
  // TEST 12: FRAGMENTED FREE SPACE
  // ============================================================

  uart_puts("\r\n--- FRAGMENTATION TEST ---\r\n");

  // Pages 0-4 occupied, page 5 empty,
  // page 6 corrupted/occupied, pages 7-15 empty.

  FLASH_TEST_REQUIRE(flash_find_next_page() == base + 5U * pageSize,
                     "Find first free page in gap");

  FLASH_TEST_REQUIRE(flash_find_free_pages(1) == base + 5U * pageSize,
                     "Find one-page free gap");

  FLASH_TEST_REQUIRE(flash_find_free_pages(2) == base + 7U * pageSize,
                     "Skip gap too small for two pages");

  FLASH_TEST_REQUIRE(flash_find_free_pages(10) == 0,
                     "Reject ten pages when no run is large enough");

  // Write another valid settings record into page 5.
  FLASH_TEST_REQUIRE(flash_write_record(settingsC, sizeof(settingsC),
                                        FLASH_STORAGE_TYPE_SETTINGS),
                     "Write settings C into fragmented space");

  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_SETTINGS, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + 5U * pageSize && sequenceNum == 3,
                     "Newest valid settings is sequence 3");

  bytesRead = 0;

  FLASH_TEST_REQUIRE(flash_read_latest_record(FLASH_STORAGE_TYPE_SETTINGS,
                                              readBuffer, sizeof(readBuffer),
                                              &bytesRead),
                     "Read settings C after corrupted record");

  FLASH_TEST_REQUIRE(
      bytesRead == headerSize + sizeof(settingsC) &&
          memcmp(readBuffer + headerSize, settingsC, sizeof(settingsC)) == 0,
      "Recovery returns settings C");

  // ============================================================
  // TEST 13: STORAGE FULL
  // ============================================================

  uart_puts("\r\n--- FULL STORAGE TEST ---\r\n");

  // All pages 0-6 are now occupied.
  // Mark each remaining page as used.

  const uint8_t marker = 0x00U;
  bool filledSuccessfully = true;

  for (uint32_t pageIndex = 7U; pageIndex < totalPages; pageIndex++) {

    uint32_t address = base + pageIndex * pageSize;

    if (!nvmctrl_write_page(address, &marker, 1U)) {
      filledSuccessfully = false;
      break;
    }
  }

  FLASH_TEST_REQUIRE(filledSuccessfully, "Fill remaining flash pages");

  FLASH_TEST_REQUIRE(flash_find_next_page() == 0, "No empty pages remain");

  FLASH_TEST_REQUIRE(flash_find_free_pages(1) == 0, "No free page available");

  FLASH_TEST_REQUIRE(!flash_write_record(settingsA, sizeof(settingsA),
                                         FLASH_STORAGE_TYPE_SETTINGS),
                     "Write correctly rejected when full");

  // Existing valid records should remain accessible.
  foundAddress = flash_find_record(FLASH_STORAGE_TYPE_SETTINGS, &sequenceNum);

  FLASH_TEST_REQUIRE(foundAddress == base + 5U * pageSize && sequenceNum == 3,
                     "Existing records survive storage-full condition");

  // ============================================================
  // COMPLETE
  // ============================================================

  uart_puts("\r\n========================================\r\n");
  uart_puts("ALL FLASH STORAGE TESTS PASSED\r\n");
  uart_puts("========================================\r\n");
}

#undef FLASH_TEST_REQUIRE