#include "test_string_utils.h"
#include "string_utils.h"
#include "uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static uint32_t testPassed;
static uint32_t testFailed;

static bool str_equals(const string *str, const char *expected) {
  if (str == NULL || str->chars == NULL || expected == NULL ||
      str->capacity == 0 || str->length >= str->capacity ||
      str->chars[str->length] != '\0' ||
      str->length != char_arr_len(expected))
    return false;

  for (uint32_t i = 0; i < str->length; i++) {
    if (str->chars[i] != expected[i])
      return false;
  }
  return true;
}

static void check_test(const char *name, bool passed) {
  uart_puts(passed ? "[PASS] " : "[FAIL] ");
  uart_puts(name);
  uart_puts("\r\n");
  if (passed)
    testPassed++;
  else
    testFailed++;
}

static bool check_exact_capacity(void) {
  char exact[6];
  string s;
  return str_init(&s, exact, sizeof(exact), "Hello") &&
         str_equals(&s, "Hello");
}

static bool check_insufficient_capacity(void) {
  char small[5];
  string s;
  return !str_init(&s, small, sizeof(small), "Hello") &&
         str_equals(&s, "");
}

static bool check_small_format(void) {
  char small[5];
  string s;
  if (!str_init(&s, small, sizeof(small), "OK"))
    return false;
  return !str_format(&s, "pre%s", "abc") && str_equals(&s, "");
}

static bool check_raw_copy(void) {
  char dst[5];
  return char_copy_char(dst, sizeof(dst), "abcd", 4) &&
         dst[0] == 'a' && dst[3] == 'd' && dst[4] == '\0';
}

static bool check_initialize_in_place(void) {
  char inPlace[8] = "Test";
  string s;
  return str_init(&s, inPlace, sizeof(inPlace), inPlace) &&
         str_equals(&s, "Test");
}

static bool check_raw_copy_capacity(void) {
  char dst[4];
  return !char_copy_char(dst, sizeof(dst), "abcd", 4);
}

void test_string_utils(void) {
  uart_puts("========= STRING UTILS TESTS =========\r\n");
  testPassed = 0;
  testFailed = 0;

  char aBuffer[64];
  char bBuffer[64];
  char outBuffer[128];
  string a;
  string b;
  string out;
  uint32_t number = 0;
  char c = '\0';

  check_test("initialize string",
             str_init(&a, aBuffer, sizeof(aBuffer), "Hello") &&
             str_equals(&a, "Hello") && str_cap(&a) == sizeof(aBuffer));

  check_test("initialize empty",
             str_init(&a, aBuffer, sizeof(aBuffer), NULL) &&
             str_equals(&a, ""));

  check_test("exact-capacity initialization", check_exact_capacity());
  check_test("reject insufficient capacity", check_insufficient_capacity());

  check_test("initialize from same buffer", check_initialize_in_place());

  check_test("reject invalid init arguments",
             !str_init(NULL, aBuffer, sizeof(aBuffer), "x") &&
             !str_init(&a, NULL, 8, "x") &&
             !str_init(&a, aBuffer, 0, "x"));

  str_init(&a, aBuffer, sizeof(aBuffer), "Hello");
  check_test("copy shorter char array",
             str_copy_char(&a, "Hi", 2) && str_equals(&a, "Hi"));

  str_init(&a, aBuffer, sizeof(aBuffer), "Hello");
  check_test("copy empty char array",
             str_copy_char(&a, "", 0) && str_equals(&a, ""));

  str_init(&a, aBuffer, sizeof(aBuffer), "Alpha");
  str_init(&b, bBuffer, sizeof(bBuffer), "XY");
  check_test("copy string",
             str_copy(&a, &b) && str_equals(&a, "XY"));

  str_init(&a, aBuffer, sizeof(aBuffer), "Hello");
  str_init(&b, bBuffer, sizeof(bBuffer), " World");
  str_init(&out, outBuffer, sizeof(outBuffer), NULL);
  check_test("concatenate strings",
             str_concat_str(&a, &b, &out) &&
             str_equals(&out, "Hello World"));

  str_init(&a, aBuffer, sizeof(aBuffer), "A");
  str_init(&b, bBuffer, sizeof(bBuffer), "BC");
  check_test("concatenate into second input",
             str_concat_str(&a, &b, &b) && str_equals(&b, "ABC"));

  str_init(&a, aBuffer, sizeof(aBuffer), "A");
  check_test("append char array",
             str_concat_char(&a, "BCDE", 3) && str_equals(&a, "ABCD"));

  check_test("reject oversized append",
             !str_concat_char(&a, "Q", 100) &&
             str_equals(&a, "ABCD"));

  str_init(&a, aBuffer, sizeof(aBuffer), "Hello World");
  str_init(&out, outBuffer, sizeof(outBuffer), NULL);
  check_test("substring end-exclusive",
             str_substring(&a, &out, 6, 11) &&
             str_equals(&out, "World"));

  check_test("empty substring",
             str_substring(&a, &out, 3, 3) && str_equals(&out, ""));

  check_test("reject invalid substring",
             !str_substring(&a, &out, 12, 14) &&
             !str_substring(&a, &out, 6, 5));

  str_init(&a, aBuffer, sizeof(aBuffer), "ABCDE");
  str_reverse(&a);
  check_test("reverse string", str_equals(&a, "EDCBA"));

  str_init(&a, aBuffer, sizeof(aBuffer), "  Hello World  ");
  str_trim(&a);
  check_test("trim multiple spaces", str_equals(&a, "Hello World"));

  str_init(&a, aBuffer, sizeof(aBuffer), "     ");
  str_trim(&a);
  check_test("trim all spaces", str_equals(&a, ""));

  str_init(&a, aBuffer, sizeof(aBuffer), "AbC!123");
  str_lower(&a);
  check_test("lowercase", str_equals(&a, "abc!123"));

  str_init(&a, aBuffer, sizeof(aBuffer), "banana");
  str_remove_char(&a, 'a');
  check_test("remove all target characters", str_equals(&a, "bnn"));

  str_init(&a, aBuffer, sizeof(aBuffer), "Hello");
  str_remove_index(&a, 1);
  check_test("remove middle index", str_equals(&a, "Hllo"));

  str_remove_index(&a, 3);
  check_test("remove final index", str_equals(&a, "Hll"));

  str_remove_index(&a, 3);
  check_test("ignore invalid remove index", str_equals(&a, "Hll"));

  check_test("get existing character",
             str_get_char(&a, &c, 1) && c == 'l' &&
             !str_get_char(&a, &c, 3));

  check_test("replace existing character",
             str_put_char(&a, 'a', 1) && str_equals(&a, "Hal") &&
             !str_put_char(&a, 'x', 3) &&
             !str_put_char(&a, '\0', 1) && str_equals(&a, "Hal"));

  check_test("index lookup",
             str_index_of(&a, 'a') == 1 &&
             str_index_of(&a, 'z') == -1);

  str_init(&a, aBuffer, sizeof(aBuffer), "0");
  check_test("parse zero",
             str_parse_uint32(&a, &number) && number == 0U);

  str_init(&a, aBuffer, sizeof(aBuffer), "0xFF");
  check_test("parse hexadecimal",
             str_parse_uint32(&a, &number) && number == 255U &&
             str_equals(&a, "0xFF"));

  str_init(&a, aBuffer, sizeof(aBuffer), "0B10101");
  check_test("parse binary",
             str_parse_uint32(&a, &number) && number == 21U);

  str_init(&a, aBuffer, sizeof(aBuffer), "0d1234");
  check_test("parse explicit decimal",
             str_parse_uint32(&a, &number) && number == 1234U);

  str_init(&a, aBuffer, sizeof(aBuffer), "4294967295");
  check_test("parse UINT32_MAX",
             str_parse_uint32(&a, &number) && number == UINT32_MAX);

  str_init(&a, aBuffer, sizeof(aBuffer), "4294967296");
  check_test("reject integer overflow", !str_parse_uint32(&a, &number));

  str_init(&a, aBuffer, sizeof(aBuffer), "0x");
  check_test("reject missing digits", !str_parse_uint32(&a, &number));

  str_init(&a, aBuffer, sizeof(aBuffer), "0b102");
  check_test("reject invalid base digit", !str_parse_uint32(&a, &number));

  str_init(&out, outBuffer, sizeof(outBuffer), NULL);
  check_test("format string and character",
             str_format(&out, "X:%s-%c-%%-Y", "Hello", 'Z') &&
             str_equals(&out, "X:Hello-Z-%-Y"));

  check_test("format unknown specifier",
             str_format(&out, "A%qB") && str_equals(&out, "A%qB"));

  check_test("format trailing percent",
             str_format(&out, "100%") && str_equals(&out, "100%"));

  check_test("format empty string",
             str_format(&out, "") && str_equals(&out, ""));

  check_test("format NULL source",
             str_format(&out, "%s", (const char *)NULL) &&
             str_equals(&out, "(null)"));

  check_test("format too-small output", check_small_format());
  check_test("raw char copy", check_raw_copy());
  check_test("raw char copy capacity", check_raw_copy_capacity());

  uart_puts(testFailed == 0 ? "STRING UTILS: ALL PASS\r\n"
                            : "STRING UTILS: FAILURES DETECTED\r\n");
}
