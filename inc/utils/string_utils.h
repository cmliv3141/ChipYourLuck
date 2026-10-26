#ifndef INC_STRING_UTILS
#define INC_STRING_UTILS

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char *chars;
  uint32_t length;
  uint32_t capacity;
} string;

uint32_t str_len(const string *str);
uint32_t char_arr_len(const char *s);
uint32_t str_cap(const string *str);

bool str_init(string *str, char *buffer, uint32_t capacity,
              const char *initialValue);

int str_index_of(const string *str, char search);
bool str_parse_uint32(const string *str, uint32_t *out);
bool str_copy(string *strTo, const string *strFrom);
bool str_copy_char(string *strTo, const char *strFrom, uint32_t length);
bool char_copy_char(char *buff, uint32_t capacity, const char *from,
                    uint32_t length);
bool str_format(string *outStr, const char *format, ...);
void str_reverse(string *str);
void str_reverse_char(char *str, uint32_t len);

bool str_concat_str(const string *str1, const string *str2, string *outStr);
bool str_concat_char(string *str, const char *s, uint32_t len);
bool str_substring(const string *str, string *outStr, uint32_t start,
                   uint32_t end);
void str_trim(string *str);
void str_lower(string *str);
bool str_get_char(const string *str, char *c, uint32_t index);
bool char_get_char(const char *buff, char *c, uint32_t index, uint32_t len);
bool char_put_char(char *buff, char value, uint32_t index, uint32_t capacity);
bool str_put_char(string *str, char c, uint32_t index);
void str_remove_char(string *str, char target);
void str_remove_index(string *str, uint32_t index);

#endif