#ifndef INC_NVMCTRL
#define INC_NVMCTRL
#include "board.h"
#include <stdbool.h>
#include <stdint.h>


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
