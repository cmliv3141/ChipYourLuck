#ifndef INC_TEST
#define INC_TEST
#include <stdbool.h>
#include <stdint.h>

void test_report(bool passed, const char *format, ...);

void test_storage_flash(void);
void test_crcc_calculation(void);
void test_flash_read_record_multi_page(void);
void test_flash_write_record_multi_page(void);
void test_flash_write_record(void);
void test_nvmctrl_get_next_page(void);
void test_nvmctrl_write_page(void);
void test_nvmctrl_erase_flash(void);
void test_nvmctrl_read_flash(void);
void test_nvmctrl_memory_info(void);
void test_ring_buffer(void);
void test_dpll0_configuration(void);
void test_nvmctrl_set_flash_wait_states(uint8_t numWaitStates);
void test_dp11_reference_clock(void);
void test_clock(void);
void test_uart(void);
void test_all(void);

#endif /* INC_TEST */
