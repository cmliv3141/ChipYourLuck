#include "utils/conversion.h"

bool int_to_string(string *outStr, int val) {
  if (outStr == NULL || outStr->chars == NULL || outStr->capacity == 0) {
    return false;
  }

  if (val == 0) {
    return str_concat_char(outStr, "0", 1);
  }

  int numDigits = 0;

  uint32_t strCap = str_cap(outStr);
  uint32_t strLen = str_len(outStr);

  uint32_t room = strCap - strLen;

  char buff[strCap];

  if (val < 0) {
    buff[0] = '-';

    val = -val;

    numDigits++;

    if (numDigits > (room)) {
      return false;
    }
  }

  while (val > 0) {
    if (numDigits > (room)) {
      return false;
    }

    int digit = val % 10;

    buff[numDigits++] = digit + '0';

    val /= 10;
  }

  str_reverse_char(&buff[0], numDigits);

  return str_concat_char(outStr, &buff[0], numDigits);
}

bool uintptr_to_hex_string(string *outStr, uintptr_t val) {
  if (outStr == NULL || outStr->chars == NULL || outStr->capacity == 0) {
    return false;
  }

  if (val == 0) {
    return str_concat_char(outStr, "0x0",
                           3);
  }

  const char *hex_digits = "0123456789ABCDEF";

  char buff[32];


  int i = sizeof(buff) - 1;
  buff[i] = '\0';


  while (val > 0) {
    i--;
    buff[i] = hex_digits[val & 0xF];
    val >>= 4;
  }


  i--;
  buff[i] = 'x';
  i--;
  buff[i] = '0';

  uint32_t numDigits = (sizeof(buff) - 1) - i;

  return str_concat_char(outStr, &buff[i], numDigits);
}

bool bool_to_string(string *outStr, bool val) {
  return val ? str_concat_char(outStr, "false", 4)
             : str_concat_char(outStr, "true", 4);
}