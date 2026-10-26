#include "utils/string_utils.h"
#include "utils/conversion.h"
#include <stdarg.h>
#include <stddef.h>

static bool str_set_length(string *str, uint32_t newLength) {
  if (str == NULL || str->chars == NULL || newLength >= str->capacity)
    return false;

  str->chars[newLength] = '\0';
  str->length = newLength;
  return true;
}

uint32_t str_len(const string *str) { return str == NULL ? 0U : str->length; }

uint32_t str_cap(const string *str) { return str == NULL ? 0U : str->capacity; }

uint32_t char_arr_len(const char *s) {
  uint32_t readIndex = 0;
  if (s == NULL)
    return 0;
  while (s[readIndex] != '\0') {
    readIndex++;
  }
  return readIndex;
}

bool str_init(string *str, char *buffer, uint32_t capacity,
              const char *initialValue) {
  if (str == NULL || buffer == NULL || capacity == 0)
    return false;

  str->chars = buffer;
  str->capacity = capacity;
  str->length = 0;

  if (initialValue == NULL) {
    buffer[0] = '\0';
    return true;
  }

  uint32_t len = char_arr_len(initialValue);
  if (len >= capacity) {
    buffer[0] = '\0';
    return false;
  }

  return str_copy_char(str, initialValue, len);
}

int str_index_of(const string *str, char search) {
  if (str == NULL || str->chars == NULL)
    return -1;

  for (uint32_t i = 0; i < str_len(str); i++) {
    char c;
    if (!str_get_char(str, &c, i))
      return -1;
    if (c == search)
      return (int)i;
  }
  return -1;
}

static bool char_isNumber(char c) { return c >= '0' && c <= '9'; }

static int str_get_base(const string *str, int *index) {
  if (str == NULL || str->chars == NULL || index == NULL || str->length == 0)
    return -1;

  char cmpChar;
  if (!str_get_char(str, &cmpChar, 0))
    return -1;

  *index = 0;
  if (!char_isNumber(cmpChar))
    return -1;

  if (cmpChar == '0' && str->length > 1) {
    if (!str_get_char(str, &cmpChar, 1))
      return -1;
    switch (cmpChar) {
    case 'x':
    case 'X':
      *index = 2;
      return 16;
    case 'b':
    case 'B':
      *index = 2;
      return 2;
    case 'd':
    case 'D':
      *index = 2;
      return 10;
    default:
      break;
    }
  }
  return 10;
}

static int char_get_number(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

bool str_parse_uint32(const string *str, uint32_t *out) {
  if (str == NULL || str->chars == NULL || out == NULL)
    return false;

  uint64_t result = 0;
  int index = -1;
  int base = str_get_base(str, &index);

  if (base == -1 || index < 0 || (uint32_t)index >= str_len(str))
    return false;

  for (uint32_t place = (uint32_t)index; place < str_len(str); place++) {
    char c;
    if (!str_get_char(str, &c, place))
      return false;

    int num = char_get_number(c);
    if (num == -1 || num >= base)
      return false;

    result = (result * (uint32_t)base) + (uint32_t)num;
    if (result > UINT32_MAX)
      return false;
  }

  *out = (uint32_t)result;
  return true;
}

bool str_format(string *outStr, const char *format, ...) {
  if (outStr == NULL || outStr->chars == NULL || outStr->capacity == 0 ||
      format == NULL)
    return false;

  va_list args;
  va_start(args, format);

  uint32_t formatIndex = 0;
  uint32_t outIndex = 0;

  while (format[formatIndex] != '\0') {
    char currentChar = format[formatIndex++];

    if (currentChar == '%') {
      char nextChar = format[formatIndex];
      if (nextChar == '\0') {
        if (outIndex >= outStr->capacity - 1U)
          goto format_fail;
        outStr->chars[outIndex++] = '%';
        break;
      }
      formatIndex++;

      switch (nextChar) {
      case 'c': {
        char c = (char)va_arg(args, int);
        if (c == '\0' || outIndex >= outStr->capacity - 1U)
          goto format_fail;
        outStr->chars[outIndex++] = c;
        break;
      }
      case 's': {
        const char *s = va_arg(args, const char *);
        if (s == NULL)
          s = "(null)";
        uint32_t len = char_arr_len(s);
        if (len > outStr->capacity - 1U - outIndex)
          goto format_fail;
        while (*s != '\0')
          outStr->chars[outIndex++] = *s++;
        break;
      }
      case '%': {
        if (outIndex >= outStr->capacity - 1U)
          goto format_fail;
        outStr->chars[outIndex++] = '%';
        break;
      }
      case 'x': {
        if ((outIndex + 10U) >= outStr->capacity - 1U)
          goto format_fail;
        uintptr_t val = (uintptr_t)va_arg(args, uintptr_t);

        string temp;
        char buffer[2 * sizeof(uintptr_t) + 4];
        if (!str_init(&temp, &buffer[0], 11, "")) {
          goto format_fail;
        }

        if (!uintptr_to_hex_string(&temp, val))
          goto format_fail;
        if (temp.length > outStr->capacity - 1U - outIndex)
          goto format_fail;

        for (uint32_t i = 0; i < temp.length; i++) {
          outStr->chars[outIndex++] = temp.chars[i];
        }

        break;
      }

      case 'd': {

        uint32_t sizeInt = (sizeof(int) * 2) + 1;

        int val = (int)va_arg(args, int);

        if (sizeInt >= outStr->capacity - 1U)
          goto format_fail;

        string temp;
        char buffer[sizeInt];

        if (!str_init(&temp, &buffer[0], sizeInt, "")) {
          goto format_fail;
        }

        if (!int_to_string(&temp, val))
          goto format_fail;
        if (temp.length > outStr->capacity - 1U - outIndex)
          goto format_fail;

        for (uint32_t i = 0; i < temp.length; i++) {
          outStr->chars[outIndex++] = temp.chars[i];
        }

        break;
      }
      default: {
        if (outStr->capacity - 1U - outIndex < 2U)
          goto format_fail;
        outStr->chars[outIndex++] = '%';
        outStr->chars[outIndex++] = nextChar;
        break;
      }
      }
    } else {
      if (outIndex >= outStr->capacity - 1U)
        goto format_fail;
      outStr->chars[outIndex++] = currentChar;
    }
  }

  va_end(args);
  return str_set_length(outStr, outIndex);

format_fail:
  va_end(args);
  str_set_length(outStr, 0);
  return false;
}

bool str_copy_char(string *strTo, const char *strFrom, uint32_t length) {
  if (strTo == NULL || strTo->chars == NULL || strFrom == NULL)
    return false;
  if (length >= str_cap(strTo))
    return false;

  for (uint32_t readIndex = 0; readIndex < length; readIndex++) {
    strTo->chars[readIndex] = strFrom[readIndex];
  }
  return str_set_length(strTo, length);
}

bool char_copy_char(char *buff, uint32_t capacity, const char *from,
                    uint32_t length) {
  if (buff == NULL || from == NULL || length >= capacity)
    return false;

  for (uint32_t readIndex = 0; readIndex < length; readIndex++) {
    if (!char_put_char(buff, from[readIndex], readIndex, capacity))
      return false;
  }
  buff[length] = '\0';
  return true;
}

bool str_copy(string *strTo, const string *strFrom) {
  if (strTo == NULL || strFrom == NULL || strFrom->chars == NULL)
    return false;
  return str_copy_char(strTo, strFrom->chars, strFrom->length);
}

bool str_concat_str(const string *str1, const string *str2, string *outStr) {
  if (str1 == NULL || str2 == NULL || outStr == NULL || str1->chars == NULL ||
      str2->chars == NULL || outStr->chars == NULL)
    return false;

  uint32_t str1Len = str_len(str1);
  uint32_t str2Len = str_len(str2);
  uint64_t totalRequiredCap = (uint64_t)str1Len + (uint64_t)str2Len;

  if (totalRequiredCap >= outStr->capacity)
    return false;

  uint32_t writeIndex = 0;
  if (outStr->chars == str2->chars) {
    for (uint32_t i = str2Len; i > 0; i--)
      outStr->chars[str1Len + i - 1U] = str2->chars[i - 1U];
    for (uint32_t i = 0; i < str1Len; i++)
      outStr->chars[i] = str1->chars[i];
    writeIndex = str1Len + str2Len;
  } else {
    for (uint32_t readIndex = 0; readIndex < str1Len; readIndex++) {
      outStr->chars[writeIndex++] = str1->chars[readIndex];
    }
    for (uint32_t readIndex = 0; readIndex < str2Len; readIndex++) {
      outStr->chars[writeIndex++] = str2->chars[readIndex];
    }
  }
  return str_set_length(outStr, writeIndex);
}

bool str_substring(const string *str, string *outStr, uint32_t start,
                   uint32_t end) {
  if (str == NULL || outStr == NULL || str->chars == NULL ||
      outStr->chars == NULL || end < start || start > str_len(str))
    return false;

  uint32_t lenStr1 = str_len(str);
  end = (end > lenStr1) ? lenStr1 : end;
  if (end - start >= str_cap(outStr))
    return false;

  uint32_t writeIndex = 0;
  for (uint32_t readIndex = start; readIndex < end; readIndex++) {
    outStr->chars[writeIndex++] = str->chars[readIndex];
  }
  return str_set_length(outStr, writeIndex);
}

void str_reverse(string *str) {
  if (str == NULL || str->chars == NULL)
    return;

  uint32_t len = str_len(str);
  if (len <= 1)
    return;

  uint32_t start = 0;
  uint32_t end = len - 1;

  while (start < end) {
    char c_start = str->chars[start];
    char c_end = str->chars[end];
    str->chars[start] = c_end;
    str->chars[end] = c_start;
    start++;
    end--;
  }
}

void str_reverse_char(char *str, uint32_t len) {

  if (str == NULL)
    return;

  if (len <= 1)
    return;

  uint32_t start = 0;
  uint32_t end = len - 1;

  while (start < end) {
    char c_start = str[start];
    char c_end = str[end];

    str[start] = c_end;
    str[end] = c_start;

    start++;
    end--;
  }
}

void str_trim(string *str) {
  if (str == NULL || str->chars == NULL)
    return;

  uint32_t newStart = 0;
  uint32_t newEnd = str_len(str);

  while (newStart < newEnd && str->chars[newStart] == ' ')
    newStart++;
  while (newEnd > newStart && str->chars[newEnd - 1] == ' ')
    newEnd--;

  uint32_t newLength = newEnd - newStart;
  for (uint32_t i = 0; i < newLength; i++) {
    str->chars[i] = str->chars[newStart + i];
  }
  str_set_length(str, newLength);
}

void str_lower(string *str) {
  if (str == NULL || str->chars == NULL)
    return;

  for (uint32_t i = 0; i < str_len(str); i++) {
    char c = str->chars[i];
    if (c >= 'A' && c <= 'Z') {
      str->chars[i] = (char)(c + ('a' - 'A'));
    }
  }
}

bool str_get_char(const string *str, char *c, uint32_t index) {
  if (str == NULL || str->chars == NULL || c == NULL || index >= str->length)
    return false;
  *c = str->chars[index];
  return true;
}

bool str_put_char(string *str, char c, uint32_t index) {
  if (str == NULL || str->chars == NULL || c == '\0' || index >= str->length)
    return false;
  str->chars[index] = c;
  return true;
}

bool str_concat_char(string *str, const char *s, uint32_t len) {
  if (str == NULL || s == NULL || str->chars == NULL)
    return false;

  uint32_t strLen = str_len(str);
  if (len > char_arr_len(s) || (uint64_t)strLen + len >= str->capacity)
    return false;

  for (uint32_t i = 0; i < len; i++) {
    str->chars[strLen++] = s[i];
  }
  return str_set_length(str, strLen);
}

bool char_get_char(const char *buff, char *c, uint32_t index, uint32_t len) {
  if (buff == NULL || c == NULL || index > len)
    return false;
  *c = buff[index];
  return true;
}

bool char_put_char(char *buff, char value, uint32_t index, uint32_t capacity) {
  if (buff == NULL || index >= capacity)
    return false;
  buff[index] = value;
  return true;
}

void str_remove_char(string *str, char target) {
  if (str == NULL || str->chars == NULL || target == '\0')
    return;

  uint32_t len = str_len(str);
  uint32_t writeIndex = 0;

  for (uint32_t readIndex = 0; readIndex < len; readIndex++) {
    char c = str->chars[readIndex];
    if (c != target) {
      str->chars[writeIndex++] = c;
    }
  }
  str_set_length(str, writeIndex);
}

void str_remove_index(string *str, uint32_t index) {
  if (str == NULL || str->chars == NULL)
    return;

  uint32_t len = str_len(str);
  if (index >= len)
    return;

  for (uint32_t readIndex = index + 1; readIndex < len; readIndex++) {
    str->chars[readIndex - 1] = str->chars[readIndex];
  }
  str_set_length(str, len - 1U);
}
