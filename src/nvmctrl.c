#include "nvmctrl.h"

// struct nvmCtrlParams {
//     uint32_t numPages;
//     uint32_t pageSize;
//     uint32_t capacity;
//     bool smartEEPROM;
// };

int nvmctrl_get_memory_info(struct nvmCtrlParams *nvmCtrl){
    uint32_t paramConfiguration = NVMCTRL_REGS->NVMCTRL_PARAM; 

    nvmCtrl->numPages = (paramConfiguration & NVMCTRL_PARAM_NVMP_Msk) >> NVMCTRL_PARAM_NVMP_Pos;
    nvmCtrl->pageSize = 8U << ((paramConfiguration & NVMCTRL_PARAM_PSZ_Msk) >> NVMCTRL_PARAM_PSZ_Pos);
    nvmCtrl->smartEEPROM = (paramConfiguration & NVMCTRL_PARAM_SEE_Msk) >> NVMCTRL_PARAM_SEE_Pos;

    nvmCtrl->capacity = nvmCtrl->numPages * nvmCtrl->pageSize;
    return 0;
}