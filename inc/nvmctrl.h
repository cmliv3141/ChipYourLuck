#ifndef INC_NVMCTRL
#define INC_NVMCTRL
#include "board.h"
#include <stdint.h>
#include <stdbool.h>

struct nvmCtrlParams {
    uint32_t numPages;
    uint32_t pageSize;
    uint32_t capacity;
    bool smartEEPROM;
};

int nvmctrl_get_memory_info(struct nvmCtrlParams *nvmCtrl);


#endif /* INC_NVMCTRL */
