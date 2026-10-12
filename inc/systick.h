#ifndef INC_SYSTICK
#define INC_SYSTICK
#include <stdint.h>

struct SysTickTimer{
    uint32_t startTime;
    uint32_t currentTime;
};


void sysTick_set_current_value(struct SysTickTimer* timer);

void sysTick_set_start_value(struct SysTickTimer* timer);

uint32_t sysTick_get_time_difference_us(struct SysTickTimer* timer);


void sysTick_init(struct SysTickTimer* timer);

#endif /* INC_SYSTICK */
