#ifndef INC_TEST
#define INC_TEST
#include <stdbool.h>
#include <stdint.h>

static void test_report(bool passed, const char *message);
void test_dpll0_configuration(void);
void test_nvmctrl_set_flash_wait_states(uint8_t numWaitStates);
void test_dp11_reference_clock(void);
void test_clock(void);
void test_uart(void);
void test_all(void);

#endif /* INC_TEST */
