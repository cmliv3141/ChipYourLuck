#include "drivers/nvmctrl.h"
#include "drivers/systick.h"

// struct nvmCtrlParams {
//     uint32_t numPages;
//     uint32_t pageSize;
//     uint32_t capacity;
//     bool smartEEPROM;
// };

static bool nvmctrl_wait_ready() {
  //create timer
  SysTickTimer timer;
  sysTick_init(&timer);

  bool timedOut = true;

  //loop and check for ready flag
  while (sysTick_get_time_difference_us(&timer) < NVM_READY_TIMEOUT_US) {
    if ((NVMCTRL_REGS->NVMCTRL_STATUS & NVMCTRL_STATUS_READY_Msk) != 0) {
      timedOut = false;
      break;
    }
  }

  //return if timedout
  return !timedOut;
}

static bool nvmctrl_wait_done(uint32_t errorMask) {

  SysTickTimer timer;
  sysTick_init(&timer);

  bool timedOut = true;

  while (sysTick_get_time_difference_us(&timer) < NVM_ERASE_TIMEOUT_US) {
    if ((NVMCTRL_REGS->NVMCTRL_INTFLAG & errorMask) != 0) {
      return false;
    }

    if ((NVMCTRL_REGS->NVMCTRL_INTFLAG & NVMCTRL_INTFLAG_DONE_Msk) != 0) {
      NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk;
      timedOut = false;
      break;
    }
  }

  return !timedOut;
}

static void nvmctrl_cmcc_invalidate(void) {

  //check if cache in enabled
  bool enabled =
      (CMCC_REGS->CMCC_CTRL & CMCC_CTRL_CEN_Msk) >> CMCC_CTRL_CEN_Pos;

  if (enabled) {

    //disable the cache
    CMCC_REGS->CMCC_CTRL &= ~CMCC_CTRL_CEN_Msk;

    //wait for it to finish
    while ((CMCC_REGS->CMCC_SR & CMCC_SR_CSTS_Msk) != 0u)
      ;
  }

  //invalidate the cache
  CMCC_REGS->CMCC_MAINT0 = CMCC_MAINT0_INVALL_Msk;

  //reenable the cache
  if (enabled) {
    CMCC_REGS->CMCC_CTRL = CMCC_CTRL_CEN_Msk;
  }

  //wait for synchronization
  __DSB();
  __ISB();
}

int nvmctrl_get_memory_info(struct nvmCtrlParams *nvmCtrl) {
  //make sure struct is defined
  if (nvmCtrl == NULL)
    return 1;

  //read the parameters from the NVM controller
  uint32_t paramConfiguration = NVMCTRL_REGS->NVMCTRL_PARAM;

  //extract the values
  nvmCtrl->numPages =
      (paramConfiguration & NVMCTRL_PARAM_NVMP_Msk) >> NVMCTRL_PARAM_NVMP_Pos;
  nvmCtrl->pageSize = 8U << ((paramConfiguration & NVMCTRL_PARAM_PSZ_Msk) >>
                             NVMCTRL_PARAM_PSZ_Pos);
  nvmCtrl->smartEEPROM =
      (paramConfiguration & NVMCTRL_PARAM_SEE_Msk) >> NVMCTRL_PARAM_SEE_Pos;

  nvmCtrl->capacity = nvmCtrl->numPages * nvmCtrl->pageSize;
  return 0;
}

bool nvmctrl_read_flash(uint32_t address, uint8_t *buff, uint32_t length) {
  //Bounds checking and data validation
  if (buff == NULL || length == 0 || address >= FLASH_SIZE ||
      length > FLASH_SIZE - address) {
    return false;
  }

  //create a byte pointer for easier data manipulation
  const uint8_t *flashPtr = (const uint8_t *)(uintptr_t)address;

  //loop through the flash reading and storing each value into the buffer
  for (uint32_t i = 0; i < length; i++) {
    buff[i] = flashPtr[i];
  }

  return true;
}

bool nvmctrl_erase_block(uint32_t blockAddress) {
  //bounds and data checking
  if (blockAddress < NVM_STORAGE_START || blockAddress >= NVM_STORAGE_END ||
      (blockAddress % NVM_BLOCK_SIZE) != 0U) {
    return false;
  }

  //wait for ready flag
  if (!nvmctrl_wait_ready())
    return false;

  //error mask with all errors we are interested in
  uint32_t errorMask = NVMCTRL_INTFLAG_ADDRE_Msk | NVMCTRL_INTFLAG_PROGE_Msk |
                       NVMCTRL_INTFLAG_LOCKE_Msk | NVMCTRL_INTFLAG_NVME_Msk;

  //clear error and done flags
  NVMCTRL_REGS->NVMCTRL_INTFLAG = errorMask | NVMCTRL_INTFLAG_DONE_Msk;

  //set the proper address
  NVMCTRL_REGS->NVMCTRL_ADDR = blockAddress;

  //send erase and execute command
  uint32_t cntrlBConfig = NVMCTRL_CTRLB_CMD_EB | NVMCTRL_CTRLB_CMDEX_KEY;

  NVMCTRL_REGS->NVMCTRL_CTRLB = cntrlBConfig;

  //wait for done and ready
  if (!nvmctrl_wait_done(errorMask))
    return false;

  if (!nvmctrl_wait_ready())
    return false;

  //check for errors
  if ((NVMCTRL_REGS->NVMCTRL_INTFLAG & errorMask) != 0)
    return false;

  //reset the done flag
  NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk;

  //create byte pointer for easier data manipulation
  const uint8_t *blockPtr = (const uint8_t *)(uintptr_t)(blockAddress);

  //verify the erase erased everything
  uint32_t index = 0;
  while (index < NVM_BLOCK_SIZE) {
    if (blockPtr[index] != 0xFF) {
      return false;
    }

    index++;
  }

  // clear cache to prevent stale cache problems
  nvmctrl_cmcc_invalidate();

  return true;
}

bool nvmctrl_write_page(uint32_t pageAddress, const uint8_t *data,
                        uint32_t len) {
  // validate data
  if (data == NULL || len == 0 || len > FLASH_PAGE_SIZE) {
    return false;
  }

  // validate address
  if (pageAddress < NVM_STORAGE_START || pageAddress >= NVM_STORAGE_END ||
      (pageAddress % FLASH_PAGE_SIZE) != 0U ||
      len > NVM_STORAGE_END - pageAddress) {
    return false;
  }

  // create the page pointer (4 byte increments)
  volatile uint32_t *pagePtr = (uint32_t *)(uintptr_t)pageAddress;

  // create a 1 byte point to make checking easier (1 byte increments)
  const uint8_t *flashPtr = (const uint8_t *)(uintptr_t)pageAddress;

  // verify the page is errased before attempting write
  for (uint32_t i = 0; i < len; i++) {
    if (flashPtr[i] != 0xFF)
      return false;
  }

  // configure CNTRLA flags
  uint32_t config = NVMCTRL_REGS->NVMCTRL_CTRLA;

  // set write mode to manual
  config &= ~NVMCTRL_CTRLA_WMODE_Msk;
  config |= NVMCTRL_CTRLA_WMODE_MAN;

  // wait for availablity of new command
  if (!nvmctrl_wait_ready())
    return false;

  // write CTRLA config
  NVMCTRL_REGS->NVMCTRL_CTRLA = config;

  // create error mask
  uint32_t errorMask = NVMCTRL_INTFLAG_ADDRE_Msk | NVMCTRL_INTFLAG_PROGE_Msk |
                       NVMCTRL_INTFLAG_LOCKE_Msk | NVMCTRL_INTFLAG_NVME_Msk;

  // clear previous flags
  NVMCTRL_REGS->NVMCTRL_INTFLAG = errorMask | NVMCTRL_INTFLAG_DONE_Msk;

  // clear page buffer
  NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_CMD_PBC | NVMCTRL_CTRLB_CMDEX_KEY;

  // wait for done flag
  if (!nvmctrl_wait_done(errorMask))
    return false;

  // wait ready
  if (!nvmctrl_wait_ready())
    return false;

  // Make sure of no errors
  if ((NVMCTRL_REGS->NVMCTRL_INTFLAG & errorMask) != 0)
    return false;

  // clear done flag
  NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk;

  // write destination address
  NVMCTRL_REGS->NVMCTRL_ADDR = pageAddress;

  // write page buffer with data
  uint32_t numWords = (len + 3U) / 4U;

  for (uint32_t writeIndex = 0; writeIndex < numWords; writeIndex++) {

    // defualt word (nothing saved)
    uint32_t word = 0xFFFFFFFFU;

    // search through the next 4 bytes of data
    for (uint32_t dataIndex = 0; dataIndex < 4U; dataIndex++) {

      uint32_t index = writeIndex * 4U + dataIndex;

      // make sure we are not access invalid memory
      if (index >= len)
        break;

      // clear the specific byte
      word &= ~(0xFFU << (8U * dataIndex));

      // store the specific byte in the word
      word |= (uint32_t)data[index] << (8U * dataIndex);
    }

    // write the word
    pagePtr[writeIndex] = word;
  }

  // wait for data synchronization
  __DSB();

  // read from the memory-mapped interface
  volatile uint32_t dummy = pagePtr[0];
  (void)dummy;

  // Page buffer is ready, time to write
  NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_CMD_WP | NVMCTRL_CTRLB_CMDEX_KEY;

  // wait for done flag
  if (!nvmctrl_wait_done(errorMask))
    return false;

  // wait ready
  if (!nvmctrl_wait_ready())
    return false;

  // Make sure of no errors
  if ((NVMCTRL_REGS->NVMCTRL_INTFLAG & errorMask) != 0)
    return false;

  // clear done flag
  NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_DONE_Msk;

  // reset the cache
  nvmctrl_cmcc_invalidate();

  // verify all contents correctly writen

  for (uint32_t i = 0; i < len; i++) {
    if (flashPtr[i] != data[i])
      return false;
  }

  return true;
}


