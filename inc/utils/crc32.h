#ifndef INC_UTILS_CRC32
#define INC_UTILS_CRC32
#include <stdint.h>

uint32_t crc32_calculate(const uint8_t *data, uint32_t length);

#endif /* INC_UTILS_CRC32 */
