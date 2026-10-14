#ifndef INC_RING_BUFFER
#define INC_RING_BUFFER
#include "board.h"
#include <stdbool.h>

struct RingBuffer {
    uint8_t array[RING_BUFFER_SIZE];
    uint8_t readIndex;
    uint8_t writeIndex;
};

void ring_buffer_init(struct RingBuffer *buff);

bool ring_buffer_push(struct RingBuffer *buff, uint8_t inByte);

bool ring_buffer_pop(struct RingBuffer *buff, uint8_t *outByte);

bool ring_buffer_is_empty(struct RingBuffer *buff);

bool ring_buffer_is_full(struct RingBuffer *buff);

#endif /* INC_RING_BUFFER */
