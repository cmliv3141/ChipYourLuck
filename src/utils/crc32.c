#include "utils/crc32.h"

#define LOWEST_BYTE_POSITION 0U
#define LOWEST_BYTE_MASK (0xFF << LOWEST_BYTE_POSITION)

uint32_t crc32_calculate(const uint8_t *data, uint32_t length)
{
    //initalize crc register
    uint32_t crcReg = 0xFFFFFFFF;

    //reflected polynominal
   const uint32_t divisor = 0xEDB88320;

    //loop through each byte
    for(uint32_t byteIndex = 0; byteIndex < length; byteIndex++)
    {
        //xor the lowest bytes
        crcReg ^= data[byteIndex];

        //loop through bits
        for(uint32_t bitIndex = 8; bitIndex > 0; bitIndex--)
        {
            crcReg = crcReg & 1 ? (crcReg >> 1) ^ divisor : crcReg >> 1;
        }
    }

    //return the inverse of crcReg
    return crcReg ^ 0xFFFFFFFF;
}