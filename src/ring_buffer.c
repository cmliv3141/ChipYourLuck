#include "ring_buffer.h"

// struct RingBuffer {
//     uint8_t array[RING_BUFFER_SIZE];
//     uint8_t readIndex;
//     uint8_t writeIndex;
// };

void ring_buffer_init(struct RingBuffer *buff) {
  buff->readIndex = 0x0;
  buff->writeIndex = 0x0;
}

bool ring_buffer_push(struct RingBuffer *buff, uint8_t inByte) {

  // Check if full
  if (!ring_buffer_is_full(buff)) {
    // store the value in the buffer
    buff->array[buff->writeIndex] = (uint8_t)inByte;

    // find if on edge
    bool edge = ((buff->writeIndex + 1) >= RING_BUFFER_SIZE);

    // advance the write index
    buff->writeIndex = (edge) ? 0 : buff->writeIndex + 1;

    return true;
  }

  return false;
}

bool ring_buffer_pop(struct RingBuffer *buff, uint8_t *outByte) {
  // Check if empty
  if (!ring_buffer_is_empty(buff)) {
    // Read the value in the buffer
    *outByte = buff->array[buff->readIndex];

    // find if on edge
    bool edge = ((buff->readIndex + 1) >= RING_BUFFER_SIZE);

    // advance the read index
    buff->readIndex = (edge) ? 0 : buff->readIndex + 1;

    return true;
  }

  return false;
}

bool ring_buffer_is_empty(struct RingBuffer *buff) {
  // check if empty
  return (buff->readIndex == buff->writeIndex);
}

bool ring_buffer_is_full(struct RingBuffer *buff) {
  // check if on edge
  bool edge = ((buff->writeIndex + 1) >= RING_BUFFER_SIZE);

  // find next valid write location
  uint8_t nextWrite = (edge) ? 0 : buff->writeIndex + 1;

  // check if full
  return (nextWrite == buff->readIndex);
}