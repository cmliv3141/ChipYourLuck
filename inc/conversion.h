#ifndef INC_CONVERSION
#define INC_CONVERSION
#include "string_utils.h"
#include "board.h"


bool int_to_string(string *outStr, int val);

bool uintptr_to_hex_string(string *outStr, uintptr_t val);

bool bool_to_string(string *outStr, bool val);

#endif /* INC_CONVERSION */
