#ifndef INC_NVMCTRL
#define INC_NVMCTRL
#include "board.h"
#include <stdbool.h>
#include <stdint.h>

#define FLASH_START 0x00000000U
#define FLASH_END 0x000FFFFFU

#define NVM_STORAGE_START 0x000FE000U
#define NVM_STORAGE_SIZE 0x00002000U
#define NVM_STORAGE_END 0x00100000U
#define NVM_BLOCK_SIZE 8192U

#define NVM_READY_TIMEOUT_US 50000U // 50 ms
#define NVM_ERASE_TIMEOUT_US 50000U // 50 ms

struct nvmCtrlParams {
  uint32_t numPages;
  uint32_t pageSize;
  uint32_t capacity;
  bool smartEEPROM;
};

int nvmctrl_get_memory_info(struct nvmCtrlParams *nvmCtrl);

bool nvmctrl_read_flash(uint32_t address, uint8_t *buff, uint32_t length);

bool nvmctrl_erase_block(uint32_t blockAddress);

bool nvmctrl_write_page(uint32_t pageAddress, const uint8_t *data,
                        uint32_t len);


#endif /* INC_NVMCTRL */
