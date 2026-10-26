#ifndef INC_SYSTICK
#define INC_SYSTICK
#include <stdint.h>

typedef struct{
    uint32_t startTime;
    uint32_t currentTime;
}SysTickTimer;


void sysTick_set_current_value(SysTickTimer* timer);

void sysTick_set_start_value(SysTickTimer* timer);

uint32_t sysTick_get_time_difference_us(SysTickTimer* timer);


void sysTick_init(SysTickTimer* timer);

#endif /* INC_SYSTICK */
